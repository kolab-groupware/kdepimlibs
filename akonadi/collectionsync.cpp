/*
    Copyright (c) 2007, 2009 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "collectionsync_p.h"
#include "collection.h"

#include "collectioncreatejob.h"
#include "collectiondeletejob.h"
#include "collectionfetchjob.h"
#include "collectionmodifyjob.h"
#include "collectionfetchscope.h"
#include "collectionmovejob.h"

#include "cachepolicy.h"

#include <kdebug.h>
#include <KLocalizedString>
#include <QtCore/QVariant>

using namespace Akonadi;

struct RemoteNode;

/**
  LocalNode is used to build a tree structure of all our locally existing collections.
*/
struct LocalNode
{
    LocalNode(const Collection &col)
        : collection(col)
        , processed(false)
        , parentNode(0)
    {}

    ~LocalNode()
    {
        qDeleteAll(childNodes);
        qDeleteAll(pendingRemoteNodes);
    }

    void appendChild(LocalNode *childNode)
    {
        childNode->parentNode = this;
        childNodes.append(childNode);
        if (!childNode->collection.remoteId().isEmpty()) {
            childRidMap.insert(childNode->collection.remoteId(), childNode);
            childNameMap.insert(childNode->collection.name(), childNode);
        }
    }

    Collection collection;
    QList<LocalNode *> childNodes;
    QHash<QString, LocalNode *> childRidMap;
    QHash<QString, LocalNode *> childNameMap;
    /** When using hierarchical RIDs we attach a list of not yet processable remote nodes to
        the closest already existing local ancestor node. They will be re-evaluated once a new
        child node is added. */
    QList<RemoteNode *> pendingRemoteNodes;
    bool processed;
    LocalNode *parentNode;
};

Q_DECLARE_METATYPE(LocalNode *)
static const char LOCAL_NODE[] = "LocalNode";

/**
  RemoteNode is used as a container for remote collections which typically don't have a UID set
  and thus cannot easily be compared or put into maps etc.
*/
struct RemoteNode
{
    RemoteNode(const Collection &col)
        : collection(col)
    {}

    Collection collection;
};

Q_DECLARE_METATYPE(RemoteNode *)
static const char REMOTE_NODE[] = "RemoteNode";

static const char CONTENTMIMETYPES[] = "CONTENTMIMETYPES";

/**
 * @internal
 */
class CollectionSync::Private
{
public:
    Private(CollectionSync *parent)
        : q(parent)
        , pendingJobs(0)
        , progress(0)
        , localRoot(0)
        , currentTransaction(0)
        , knownLocalCollections(0)
        , incremental(false)
        , streaming(false)
        , hierarchicalRIDs(false)
        , localListDone(false)
        , deliveryDone(false)
    {
    }

    ~Private()
    {
        delete localRoot;
        qDeleteAll(rootRemoteNodes);
    }

    /** Utility method to reset the node tree. */
    void resetNodeTree()
    {
        delete localRoot;
        localRoot = new LocalNode(Collection::root());
        localRoot->parentNode = 0;
        localRoot->processed = true; // never try to delete that one
        if (currentTransaction) {
            // we are running the update transaction, initialize pending remote nodes
            localRoot->pendingRemoteNodes.swap(rootRemoteNodes);
        }

        localUidMap.clear();
        localRidMap.clear();
        localUidMap.insert(localRoot->collection.id(), localRoot);
        localRidMap.insert(QString(), localRoot);
    }

    /** Create a local node from the given local collection and integrate it into the local tree structure. */
    LocalNode *createLocalNode(const Collection &col)
    {
        LocalNode *node = new LocalNode(col);
        Q_ASSERT(!localUidMap.contains(col.id()));
        localUidMap.insert(node->collection.id(), node);
        if (!col.remoteId().isEmpty()) {
            if (!hierarchicalRIDs) {
                localRidMap.insert(node->collection.remoteId(), node);
            } else {
                localRidMap.insertMulti(node->collection.remoteId(), node);
            }
        }

        // add already existing children
        if (localPendingCollections.contains(col.id())) {
            QVector<Collection::Id> childIds = localPendingCollections.take(col.id());
            foreach (Collection::Id childId, childIds) {
                Q_ASSERT(localUidMap.contains(childId));
                LocalNode *childNode = localUidMap.value(childId);
                node->appendChild(childNode);
            }
        }

        // set our parent and add ourselves as child
        if (localUidMap.contains(col.parentCollection().id())) {
            LocalNode *parentNode = localUidMap.value(col.parentCollection().id());
            parentNode->appendChild(node);
        } else {
            localPendingCollections[col.parentCollection().id()].append(col.id());
        }

        return node;
    }

    /** Same as createLocalNode() for remote collections. */
    void createRemoteNode(const Collection &col)
    {
        if (col.remoteId().isEmpty()) {
            kWarning() << "Collection '" << col.name() << "' does not have a remote identifier - skipping";
            return;
        }
        RemoteNode *node = new RemoteNode(col);
        rootRemoteNodes.append(node);
    }

    /** Create local nodes as we receive the local listing from the Akonadi server. */
    void localCollectionsReceived(const Akonadi::Collection::List &localCols)
    {
        foreach (const Collection &c, localCols) {
            createLocalNode(c);
            knownLocalCollections++;
        }
    }

    /** Once the local collection listing finished we can continue with the interesting stuff. */
    void localCollectionFetchResult(KJob *job)
    {
        if (job->error()) {
            return; // handled by the base class
        }

        // safety check: the local tree has to be connected
        if (!localPendingCollections.isEmpty()) {
            q->setError(Unknown);
            q->setErrorText(i18n("Inconsistent local collection tree detected."));
            q->emitResult();
            return;
        }

        localListDone = true;
        execute();
    }

    /**
     * Find a child node with matching collection name.
     * @note This is used as a fallback if the resource lost the RID update somehow.
     * This can be used because the Akonadi server enforces unique child collection names inside the hierarchy
     */
    LocalNode *findLocalChildNodeByName(LocalNode *localParentNode, const QString &name) const
    {
        if (name.isEmpty()) {   // shouldn't happen...
            return 0;
        }

        if (localParentNode == localRoot) {   // possibly non-unique names on top-level
            return 0;
        }
        LocalNode *childNode = localParentNode->childNameMap.value(name, 0);
        // the restriction on empty RIDs can possibly removed, but for now I only understand the implication for this case
        if (childNode && childNode->collection.remoteId().isEmpty()) {
            return childNode;
        }

        return 0;
    }

    template<typename T>
    QPair<LocalNode * /*node*/, LocalNode * /*parent*/> findMatchingLocalNodeFromCandidates(const Collection &collection, const T &candidates) const
    {
        QSet<LocalNode *> suitableParents;
        const Collection parentCollection = collection.parentCollection();
        const QString parentRid = parentCollection.remoteId();
        const Collection root = Collection::root();
        Q_FOREACH (LocalNode *candidate, candidates) {
            if (candidate->parentNode) {
                LocalNode *parentNode = candidate->parentNode;
                if (parentNode->collection.remoteId() == parentRid) {
                    suitableParents.insert(parentNode);
                }
            } else if (parentCollection.id() == root.id() || parentRid == root.remoteId()) {
                suitableParents.insert(localRoot);
            } else {
                /* not a suitable candidate */
            }
        }

        LocalNode *parentNode = 0;
        if (suitableParents.isEmpty()) {
            return qMakePair<LocalNode *, LocalNode *>(0, 0);
        } else if (suitableParents.count() == 1) {
            // TODO: Should we try to match the candidate by the entire ancestor chain?
            parentNode = *suitableParents.begin();
        } else {
            const QPair<LocalNode *, LocalNode *> pair = findMatchingLocalNodeFromCandidates(parentCollection, suitableParents);
            parentNode = pair.first;
        }

        if (parentNode) {
            return qMakePair(parentNode->childRidMap.value(collection.remoteId(), 0), parentNode);
        }

        return qMakePair<LocalNode *, LocalNode *>(0, 0);
    }


    /**
      Find the local node that matches the given remote collection, returns 0
      if that doesn't exist (yet).
    */
    QPair<LocalNode * /*node*/, LocalNode * /*parent*/> findMatchingLocalNode(const Collection &collection) const
    {
        if (!hierarchicalRIDs) {
            LocalNode *node = localRidMap.value(collection.remoteId(), 0);
            return qMakePair(node, node ? node->parentNode : localRidMap.value(collection.parentRemoteId(), 0));
        } else {
            if (collection.id() == Collection::root().id() || collection.remoteId() == Collection::root().remoteId()) {
                return qMakePair(localRoot, (LocalNode *)0);
            }

            const QList<LocalNode*> candidates = localRidMap.values(collection.remoteId());
            if (!candidates.isEmpty()) {
                const QPair<LocalNode *, LocalNode *> pair = findMatchingLocalNodeFromCandidates(collection, candidates);
                if (pair.first && pair.second) {
                    return pair;
                }
            }

            const QPair<LocalNode *, LocalNode *> pair = findMatchingLocalNode(collection.parentCollection());
            LocalNode *localParent = pair.first;
            if (!localParent) {
                return qMakePair<LocalNode *, LocalNode *>(0, 0);
            }

            if (LocalNode *recoveredLocalNode = findLocalChildNodeByName(localParent, collection.name())) {
                kDebug() << "Recovering collection with lost RID:" << collection << recoveredLocalNode->collection;
                return qMakePair(recoveredLocalNode, localParent);
            }

            return qMakePair((LocalNode *)0, localParent);
        }
    }

    /**
      Find the local node that is the nearest ancestor of the given remote collection
      (when using hierarchical RIDs only, otherwise it's always the local root node).
      Never returns 0.
    */
    LocalNode *findBestLocalAncestor(const Collection &collection, bool *exactMatch = 0)
    {
        if (!hierarchicalRIDs) {
            return localRoot;
        }
        if (collection == Collection::root()) {
            if (exactMatch) {
                *exactMatch = true;
            }
            return localRoot;
        }
        const Collection parentCollection = collection.parentCollection();
        if (parentCollection.id() < 0 && parentCollection.remoteId().isEmpty()) {
            kWarning() << "Remote collection without valid parent found: " << collection;
            return 0;
        }
        bool parentIsExact = false;
        LocalNode *localParent = findBestLocalAncestor(parentCollection, &parentIsExact);
        if (!parentIsExact) {
            if (exactMatch) {
                *exactMatch = false;
            }
            return localParent;
        }
        LocalNode *childNode = localParent->childRidMap.value(collection.remoteId(), 0);
        if (childNode) {
            if (exactMatch) {
                *exactMatch = true;
            }
            return childNode;
        }
        if (exactMatch) {
            *exactMatch = false;
        }
        return localParent;
    }

    /**
      Checks if any of the remote nodes is not equal to the current local one. If so return true.
    */
    bool checkPendingRemoteNodes() const
    {
        if (rootRemoteNodes.size() != knownLocalCollections) {
            return true;
        }

        foreach (RemoteNode *remoteNode, rootRemoteNodes) {
            // every remote note should have a local node already
            const QPair<LocalNode *, LocalNode *> nodePair = findMatchingLocalNode(remoteNode->collection);
            const LocalNode *localNode = nodePair.first;
            if (localNode) {
                if (checkLocalCollection(localNode, remoteNode)) {
                    return true;
                }
            } else {
                return true;
            }
        }
        return false;
    }

    /**
      Checks the pending remote nodes attached to the given local root node
      to see if any of them can be processed by now. If not, they are moved to
      the closest ancestor available.
    */
    void processPendingRemoteNodes(LocalNode *_localRoot)
    {
        QList<RemoteNode *> pendingRemoteNodes(_localRoot->pendingRemoteNodes);
        _localRoot->pendingRemoteNodes.clear();
        QHash<LocalNode *, QList<RemoteNode *> > pendingCreations;
        foreach (RemoteNode *remoteNode, pendingRemoteNodes) {
            // step 1: see if we have a matching local node already
            const QPair<LocalNode *, LocalNode *> nodes = findMatchingLocalNode(remoteNode->collection);
            LocalNode *localNode = nodes.first;
            if (localNode) {
                Q_ASSERT(!localNode->processed);
                updateLocalCollection(localNode, remoteNode);
                continue;
            }
            // step 2: check if we have the parent at least, then we can create it
            localNode = nodes.second;
            if (localNode) {
                pendingCreations[localNode].append(remoteNode);
                continue;
            }
            // step 3: find the best matching ancestor and enqueue it for later processing
            localNode = findBestLocalAncestor(remoteNode->collection);
            if (!localNode) {
                q->setError(Unknown);
                q->setErrorText(i18n("Remote collection without root-terminated ancestor chain provided, resource is broken."));
                q->emitResult();
                return;
            }
            localNode->pendingRemoteNodes.append(remoteNode);
        }

        // process the now possible collection creations
        for (QHash<LocalNode *, QList<RemoteNode *> >::const_iterator it = pendingCreations.constBegin();
             it != pendingCreations.constEnd(); ++it) {
            createLocalCollections(it.key(), it.value());
        }
    }

    /**
      Checks if the given localNode and remoteNode are different
    */
    bool checkLocalCollection(const LocalNode *localNode, const RemoteNode *remoteNode) const
    {
        const Collection &localCollection = localNode->collection;
        const Collection &remoteCollection = remoteNode->collection;

        if (!keepLocalChanges.contains(CONTENTMIMETYPES) && !remoteCollection.keepLocalChanges().contains(CONTENTMIMETYPES)) {
            if (localCollection.contentMimeTypes().size() != remoteCollection.contentMimeTypes().size()) {
                return true;
            } else {
                for (int i = 0; i < remoteCollection.contentMimeTypes().size(); i++) {
                    const QString &m = remoteCollection.contentMimeTypes().at(i);
                    if (!localCollection.contentMimeTypes().contains(m)) {
                        return true;
                    }
                }
            }
        }

        if (localCollection.parentCollection().remoteId() != remoteCollection.parentCollection().remoteId()) {
            return true;
        }
        if (localCollection.name() != remoteCollection.name()) {
            return true;
        }
        if (localCollection.remoteId() != remoteCollection.remoteId()) {
            return true;
        }
        if (localCollection.remoteRevision() != remoteCollection.remoteRevision()) {
            return true;
        }
        if (!(localCollection.cachePolicy() == remoteCollection.cachePolicy())) {
            return true;
        }
        if (localCollection.enabled() != remoteCollection.enabled()) {
            return true;
        }

        // CollectionModifyJob adds the remote attributes to the local collection
        foreach (const Attribute *attr, remoteCollection.attributes()) {
            const Attribute *localAttr = localCollection.attribute(attr->type());
            if (localAttr && (keepLocalChanges.contains(attr->type()) || remoteCollection.keepLocalChanges().contains(CONTENTMIMETYPES))) {
                continue;
            }
            // The attribute must both exist and have equal contents
            if (!localAttr || localAttr->serialized() != attr->serialized()) {
                return true;
            }
        }

        return false;
    }

    /**
      Performs a local update for the given node pair.
    */
    void updateLocalCollection(LocalNode *localNode, RemoteNode *remoteNode)
    {
        Collection upd(remoteNode->collection);
        Q_ASSERT(!upd.remoteId().isEmpty());
        Q_ASSERT(currentTransaction);
        upd.setId(localNode->collection.id());
        if (keepLocalChanges.contains(CONTENTMIMETYPES) || remoteNode->collection.keepLocalChanges().contains(CONTENTMIMETYPES)) {
            upd.setContentMimeTypes(localNode->collection.contentMimeTypes());
        }
        foreach (Attribute *remoteAttr, upd.attributes()) {
            if ((keepLocalChanges.contains(remoteAttr->type()) || remoteNode->collection.keepLocalChanges().contains(remoteAttr->type()))&& localNode->collection.hasAttribute(remoteAttr->type())) {
                //We don't want to overwrite the attribute changes with the defaults provided by the resource.
                Attribute *localAttr = localNode->collection.attribute(remoteAttr->type());
                upd.removeAttribute(localAttr->type());
                upd.addAttribute(localAttr->clone());
            }
        }

        if (checkLocalCollection(localNode, remoteNode)) {
            // ### HACK to work around the implicit move attempts of CollectionModifyJob
            // which we do explicitly below
            Collection c(upd);
            c.setParentCollection(localNode->collection.parentCollection());
            ++pendingJobs;
            CollectionModifyJob *mod = new CollectionModifyJob(c, currentTransaction);
            connect(mod, SIGNAL(result(KJob*)), q, SLOT(updateLocalCollectionResult(KJob*)));
        }

        // detecting moves is only possible with global RIDs
        if (!hierarchicalRIDs) {
            const LocalNode *oldParent = localUidMap.value(localNode->collection.parentCollection().id());
            const QPair<LocalNode *, LocalNode *> nodes = findMatchingLocalNode(remoteNode->collection.parentCollection());
            const LocalNode *newParent = nodes.first;
            // TODO: handle the newParent == 0 case correctly, ie. defer the move until the new
            // local parent has been created
            if (newParent && oldParent != newParent) {
                ++pendingJobs;
                CollectionMoveJob *move = new CollectionMoveJob(upd, newParent->collection, currentTransaction);
                //move->setProperty("collection", QVariant::fromValue(upd));
                connect(move, SIGNAL(result(KJob*)), q, SLOT(updateLocalCollectionResult(KJob*)));
            }
        }

        localNode->processed = true;
        delete remoteNode;
    }

    void updateLocalCollectionResult(KJob *job)
    {
        --pendingJobs;
        if (job->error()) {
            return; // handled by the base class
        }
        if (qobject_cast<CollectionModifyJob *>(job)) {
            //const Collection col = qobject_cast<CollectionModifyJob*>(job)->collection();
            //qDebug() << "== Updated collection" << col.parentRemoteId() << col.remoteId();
            ++progress;
        } else {
            //const Collection col = job->property("collection").value<Collection>();
            //qDebug() << "== Moved collection" << col.parentRemoteId() << col.remoteId();
        }
        checkDone();
    }

    /**
      Creates local folders for the given local parent and remote nodes.
      @todo group CollectionCreateJobs into a single one once it supports that
    */
    void createLocalCollections(LocalNode *localParent, QList<RemoteNode *> remoteNodes)
    {
        foreach (RemoteNode *remoteNode, remoteNodes) {
            ++pendingJobs;
            Collection col(remoteNode->collection);
            Q_ASSERT(!col.remoteId().isEmpty());
            col.setParentCollection(localParent->collection);
            CollectionCreateJob *create = new CollectionCreateJob(col, currentTransaction);
            create->setProperty(LOCAL_NODE, QVariant::fromValue(localParent));
            create->setProperty(REMOTE_NODE, QVariant::fromValue(remoteNode));
            connect(create, SIGNAL(result(KJob*)), q, SLOT(createLocalCollectionResult(KJob*)));

            // Commit transaction after every 100 collections are created,
            // otherwise it overlads database journal and things get veeery slow
            if (pendingJobs % 50 == 0) {
                currentTransaction->commit();
                createTransaction();
            }
        }
    }

    void createLocalCollectionResult(KJob *job)
    {
        --pendingJobs;
        if (job->error()) {
            return; // handled by the base class
        }

        const Collection newLocal = static_cast<CollectionCreateJob *>(job)->collection();
        //qDebug() << "== Created collection" << newLocal.parentRemoteId() << newLocal.remoteId();
        LocalNode *localNode = createLocalNode(newLocal);
        localNode->processed = true;

        LocalNode *localParent = job->property(LOCAL_NODE).value<LocalNode *>();
        Q_ASSERT(localParent->childNodes.contains(localNode));
        RemoteNode *remoteNode = job->property(REMOTE_NODE).value<RemoteNode *>();
        delete remoteNode;
        ++progress;

        processPendingRemoteNodes(localParent);
        if (!hierarchicalRIDs) {
            processPendingRemoteNodes(localRoot);
        }

        checkDone();
    }

    /**
      Checks if the given local node has processed child nodes.
    */
    bool hasProcessedChildren(LocalNode *localNode) const
    {
        if (localNode->processed) {
            return true;
        }
        foreach (LocalNode *child, localNode->childNodes) {
            if (hasProcessedChildren(child)) {
                return true;
            }
        }
        return false;
    }

    /**
      Find all local nodes that are not marked as processed and have no children that
      are marked as processed.
    */
    Collection::List findUnprocessedLocalCollections(LocalNode *localNode) const
    {
        Collection::List rv;
        if (!localNode->processed) {
            if (hasProcessedChildren(localNode)) {
                kWarning() << "Found unprocessed local node with processed children, excluding from deletion";
                kWarning() << localNode->collection;
                return rv;
            }
            if (localNode->collection.remoteId().isEmpty()) {
                kWarning() << "Found unprocessed local node without remoteId, excluding from deletion";
                kWarning() << localNode->collection;
                return rv;
            }
            rv.append(localNode->collection);
            return rv;
        }

        foreach (LocalNode *child, localNode->childNodes) {
            rv.append(findUnprocessedLocalCollections(child));
        }
        return rv;
    }

    /**
      Deletes unprocessed local nodes, in non-incremental mode.
    */
    void deleteUnprocessedLocalNodes()
    {
        if (incremental) {
            return;
        }
        const Collection::List cols = findUnprocessedLocalCollections(localRoot);
        deleteLocalCollections(cols);
    }

    /**
      Deletes the given collection list.
      @todo optimize delete job to support batch operations
    */
    void deleteLocalCollections(const Collection::List &cols)
    {
        q->setTotalAmount(KJob::Bytes, q->totalAmount(KJob::Bytes) + cols.size());
        foreach (const Collection &col, cols) {
            Q_ASSERT(!col.remoteId().isEmpty());   // empty RID -> stuff we haven't even written to the remote side yet

            ++pendingJobs;
            Q_ASSERT(currentTransaction);
            CollectionDeleteJob *job = new CollectionDeleteJob(col, currentTransaction);
            //job->setProperty("collection", QVariant::fromValue(col));
            connect(job, SIGNAL(result(KJob*)), q, SLOT(deleteLocalCollectionsResult(KJob*)));

            // It can happen that the groupware servers report us deleted collections
            // twice, in this case this collection delete job will fail on the second try.
            // To avoid a rollback of the complete transaction we gracefully allow the job
            // to fail :)
            currentTransaction->setIgnoreJobFailure(job);
        }
    }

    void deleteLocalCollectionsResult(KJob *job)
    {
        Q_UNUSED(job);

        --pendingJobs;

        //const Collection col = job->property("collection").value<Collection>();
        //qDebug() << "== Deleted collection" << col.parentRemoteId() << col.remoteId();
        ++progress;
        checkDone();
    }

    void createTransaction()
    {
        currentTransaction = new TransactionSequence(q);
        currentTransaction->setAutomaticCommittingEnabled(false);
        q->connect(currentTransaction, SIGNAL(finished(KJob*)),
                   q, SLOT(transactionSequenceResult(KJob*)));
    }

    /**
      Check update necessity.
    */
    void checkUpdateNecessity()
    {
        bool updateNeeded = checkPendingRemoteNodes();
        if (!updateNeeded) {
            // We can end right now
            q->emitResult();
            return;
        }

        // Since there are differences with the remote collections we need to sync. Start a transaction here.
        Q_ASSERT(!currentTransaction);
        createTransaction();

        // Now that a transaction is started we need to fetch local collections again and do the update
        q->doStart();
    }

    /** After the transaction has finished report we're done as well. */
    void transactionSequenceResult(KJob *job)
    {
        if (job->error()) {
            return; // handled by the base class
        }

        // If this was the last transaction, then finish, otherwise there's
        // a new transaction in the queue already
        if (job == currentTransaction) {
            q->emitResult();
        }
    }

    /**
      Process what's currently available.
    */
    void execute()
    {
        kDebug() << Q_FUNC_INFO << "localListDone: " << localListDone << " deliveryDone: " << deliveryDone;
        if (!localListDone || !deliveryDone) {
            return;
        }

        // If a transaction is not started yet we are still checking if the update is
        // actually needed.
        if (!currentTransaction) {
            checkUpdateNecessity();
            return;
        }

        // Since the transaction is already running we need to execute the update.
        processPendingRemoteNodes(localRoot);

        if (!incremental && deliveryDone) {
            deleteUnprocessedLocalNodes();
        }

        if (!hierarchicalRIDs) {
            deleteLocalCollections(removedRemoteCollections);
        } else {
            Collection::List localCols;
            foreach (const Collection &c, removedRemoteCollections) {
                const QPair<LocalNode *, LocalNode *> nodePair = findMatchingLocalNode(c);
                const LocalNode *node = nodePair.first;
                if (node) {
                    localCols.append(node->collection);
                }
            }
            deleteLocalCollections(localCols);
        }
        removedRemoteCollections.clear();

        checkDone();
    }

    /**
      Finds pending remote nodes, which at the end of the day should be an empty set.
    */
    QList<RemoteNode *> findPendingRemoteNodes(LocalNode *localNode)
    {
        QList<RemoteNode *> rv;
        rv.append(localNode->pendingRemoteNodes);
        foreach (LocalNode *child, localNode->childNodes) {
            rv.append(findPendingRemoteNodes(child));
        }
        return rv;
    }

    /**
      Are we there yet??
      @todo progress reporting
    */
    void checkDone()
    {
        q->setProcessedAmount(KJob::Bytes, progress);

        // still running jobs or not fully delivered local/remote state
        if (!deliveryDone || pendingJobs > 0 || !localListDone) {
            return;
        }

        // safety check: there must be no pending remote nodes anymore
        QList<RemoteNode *> orphans = findPendingRemoteNodes(localRoot);
        if (!orphans.isEmpty()) {
            q->setError(Unknown);
            q->setErrorText(i18n("Found unresolved orphan collections"));
            foreach (RemoteNode *orphan, orphans) {
                kDebug() << "found orphan collection:" << orphan->collection;
            }
            q->emitResult();
            return;
        }

        kDebug() << Q_FUNC_INFO << "q->commit()";
        Q_ASSERT(currentTransaction);
        currentTransaction->commit();
    }

    CollectionSync *q;

    QString resourceId;

    int pendingJobs;
    int progress;

    LocalNode *localRoot;
    TransactionSequence *currentTransaction;
    QHash<Collection::Id, LocalNode *> localUidMap;
    QHash<QString, LocalNode *> localRidMap;

    // temporary during build-up of the local node tree, must be empty afterwards
    QHash<Collection::Id, QVector<Collection::Id> > localPendingCollections;

    // removed remote collections in incremental mode
    Collection::List removedRemoteCollections;

    // used to store the list of remote nodes passed by the user
    QList<RemoteNode *> rootRemoteNodes;

    // keep track of the total number of local collections that are known
    // only used during the preliminary check to see if updating is needed
    int knownLocalCollections;

    bool incremental;
    bool streaming;
    bool hierarchicalRIDs;

    bool localListDone;
    bool deliveryDone;

    // List of parts where local changes should not be overwritten
    QSet<QByteArray> keepLocalChanges;
};

CollectionSync::CollectionSync(const QString &resourceId, QObject *parent)
    : Job(parent)
    , d(new Private(this))
{
    d->resourceId = resourceId;
    setTotalAmount(KJob::Bytes, 0);
}

CollectionSync::~CollectionSync()
{
    delete d;
}

void CollectionSync::setRemoteCollections(const Collection::List &remoteCollections)
{
    setTotalAmount(KJob::Bytes, totalAmount(KJob::Bytes) + remoteCollections.count());
    foreach (const Collection &c, remoteCollections) {
        d->createRemoteNode(c);
    }

    if (!d->streaming) {
        d->deliveryDone = true;
    }
    d->execute();
}

void CollectionSync::setRemoteCollections(const Collection::List &changedCollections, const Collection::List &removedCollections)
{
    setTotalAmount(KJob::Bytes, totalAmount(KJob::Bytes) + changedCollections.count());
    d->incremental = true;
    foreach (const Collection &c, changedCollections) {
        d->createRemoteNode(c);
    }
    d->removedRemoteCollections += removedCollections;

    if (!d->streaming) {
        d->deliveryDone = true;
    }
    d->execute();
}

void CollectionSync::doStart()
{
    d->resetNodeTree();
    d->knownLocalCollections = 0;
    Job *parent = (d->currentTransaction ? static_cast<Job *>(d->currentTransaction) : static_cast<Job *>(this));
    CollectionFetchJob *job = new CollectionFetchJob(Collection::root(), CollectionFetchJob::Recursive, parent);
    job->fetchScope().setResource(d->resourceId);
    job->fetchScope().setIncludeUnsubscribed(true);
    job->fetchScope().setAncestorRetrieval(CollectionFetchScope::Parent);
    connect(job, SIGNAL(collectionsReceived(Akonadi::Collection::List)),
            SLOT(localCollectionsReceived(Akonadi::Collection::List)));
    connect(job, SIGNAL(result(KJob*)), SLOT(localCollectionFetchResult(KJob*)));
}

void CollectionSync::setStreamingEnabled(bool streaming)
{
    d->streaming = streaming;
}

void CollectionSync::retrievalDone()
{
    d->deliveryDone = true;
    d->execute();
}

void CollectionSync::setHierarchicalRemoteIds(bool hierarchical)
{
    d->hierarchicalRIDs = hierarchical;
}

void CollectionSync::rollback()
{
    if (d->currentTransaction) {
        d->currentTransaction->rollback();
    }
}

void CollectionSync::setKeepLocalChanges(const QSet<QByteArray> &parts)
{
    d->keepLocalChanges = parts;
}

#include "moc_collectionsync_p.cpp"
