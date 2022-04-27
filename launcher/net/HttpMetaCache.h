/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <qtimer.h>
#include <QMap>
#include <QString>
#include <memory>

class HttpMetaCache;

class MetaEntry {
    friend class HttpMetaCache;

   protected:
    MetaEntry() = default;

   public:
    auto isStale() -> bool { return stale; }
    void setStale(bool stale) { this->stale = stale; }

    auto getFullPath() -> QString;

    auto getRemoteChangedTimestamp() -> QString { return remote_changed_timestamp; }
    void setRemoteChangedTimestamp(QString remote_changed_timestamp) { this->remote_changed_timestamp = remote_changed_timestamp; }
    void setLocalChangedTimestamp(qint64 timestamp) { local_changed_timestamp = timestamp; }

    auto getETag() -> QString { return etag; }
    void setETag(QString etag) { this->etag = etag; }

    auto getMD5Sum() -> QString { return md5sum; }
    void setMD5Sum(QString md5sum) { this->md5sum = md5sum; }

   protected:
    QString baseId;
    QString basePath;
    QString relativePath;
    QString md5sum;
    QString etag;
    qint64 local_changed_timestamp = 0;
    QString remote_changed_timestamp;  // QString for now, RFC 2822 encoded time
    bool stale = true;
};

using MetaEntryPtr = std::shared_ptr<MetaEntry>;

class HttpMetaCache : public QObject {
    Q_OBJECT
   public:
    // supply path to the cache index file
    HttpMetaCache(QString path = QString());
    ~HttpMetaCache() override;

    // get the entry solely from the cache
    // you probably don't want this, unless you have some specific caching needs.
    auto getEntry(QString base, QString resource_path) -> MetaEntryPtr;

    // get the entry from cache and verify that it isn't stale (within reason)
    auto resolveEntry(QString base, QString resource_path, QString expected_etag = QString()) -> MetaEntryPtr;

    // add a previously resolved stale entry
    auto updateEntry(MetaEntryPtr stale_entry) -> bool;

    // evict selected entry from cache
    auto evictEntry(MetaEntryPtr entry) -> bool;

    void addBase(QString base, QString base_root);

    // (re)start a timer that calls SaveNow later.
    void SaveEventually();
    void Load();

    auto getBasePath(QString base) -> QString;

   public slots:
    void SaveNow();

   private:
    // create a new stale entry, given the parameters
    auto staleEntry(QString base, QString resource_path) -> MetaEntryPtr;

    struct EntryMap {
        QString base_path;
        QMap<QString, MetaEntryPtr> entry_list;
    };

    QMap<QString, EntryMap> m_entries;
    QString m_index_file;
    QTimer saveBatchingTimer;
};
