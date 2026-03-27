#include "filedownloader.h"
#include "Logger.h"

FileDownloader::FileDownloader(QUrl imageUrl, QObject *parent) :
    QObject(parent)
{
    connect(&m_WebCtrl, &QNetworkAccessManager::finished,
                this, &FileDownloader::fileDownloaded);

    QNetworkRequest request(imageUrl);
    m_WebCtrl.get(request);
}

FileDownloader::~FileDownloader()
{

}

void FileDownloader::fileDownloaded(QNetworkReply* pReply)
{
    if (pReply->error() == QNetworkReply::NoError) {
        m_DownloadedData = pReply->readAll();
    } else {
        Logger::instance().error(QString("Download failed: %1").arg(pReply->errorString()));
        m_DownloadedData.clear();
    }
    pReply->deleteLater();
    emit downloaded();
}

QByteArray FileDownloader::downloadedData() const
{
    return m_DownloadedData;
}
