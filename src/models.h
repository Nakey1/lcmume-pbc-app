#pragma once
#include <QString>

struct PublicParams {
    QString paramStr;
    QString G;
    QString Ppub;
};

struct PartialKey {
    QString userId;
    QString d;
    QString R;
};

struct PeerPubKeys {
    QString userId;
    QString targetId;
    QString X;
    QString R;
};
