#include "Octree.h"
#include <QtCore>
#include <QtGui>
#include "Utils.h"

Octree::Octree(QObject* parent) : QObject(parent) {
    m_worldToOctree = glm::inverse(m_octreeToWorld);
}

void Octree::createNew() {
    source.create();
    storage = source.binary();

    /* Example
     *
    // Header
    storage.append(0x00000002); // => Block info
    // Nodes
    storage.append(0x0000FF00); // 0000 0000 0000 0000 | 1111 1111 | 0000 0000
    // Block info
    storage.append(0x00000000);
    // Attach data
    storage.append(0x000001FF); // 0000 0000 0000 0000 0000 0001 | 1111 1111
    // Colors
    storage.append(defaultColor);
    storage.append(defaultColor);
    storage.append(defaultColor);
    storage.append(defaultColor);
    storage.append(defaultColor);
    storage.append(defaultColor);
    storage.append(defaultColor);
    storage.append(defaultColor);
    */
}

void Octree::createTest() {
    /*
    storage.clear();
    // Test data
    // Header
    storage.append(0x00000003); // => Block info
    // Nodes
    storage.append(0x00027F08); // 0000 0000 0000 0010 | 0111 1111 | 0000 1000
    storage.append(0x0000BF00); // 0000 0000 0000 0000 | 1101 1111 | 0000 0000
    // Block info
    storage.append(0x00000000);
    // Attach data
    storage.append(0x0000027F); // 0000 0000 0000 0000 0000 0002 | 0111 1111
    storage.append(0x00000999); // 0000 0000 0000 0000 0000 1001 | 1001 1001
    // Colors
    storage.append(0x044A00FF);
    storage.append(0xA30000FF);
    storage.append(0x13DAFFFF);
    storage.append(0x7E4681FF);
    storage.append(0xFF0011FF);
    storage.append(0x903472FF);
    storage.append(0xAD7D4DFF);

    storage.append(0x5A1681FF);
    storage.append(0x6B38ACFF);
    storage.append(0x5DD772FF);
    storage.append(0xC5651FFF);
    */
}

bool Octree::save(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "Could not open file for writing";
        return false;
    }

    QTextStream out(&file);
    out << source.serialize();
    file.close();

    return true;
}

bool Octree::load(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "Could not open file for reading";
        return false;
    }
    QTextStream in(&file);
    QString data = in.readAll();
    source.create(data);

    file.close();

    return true;
}

void Octree::setIsModified(bool isModified) {
    if (isModified == m_isModified) return;
    m_isModified = isModified;
    emit isModifiedChanged(isModified);
}

int Octree::colorAttachAddress(int parent, int childIndex) {
    int pageHeader = parent & -pageBytes;
    int blockInfo = pageHeader + storage->at(pageHeader);
    int attachData = blockInfo + blockInfoEnd;
    int paletteNode = storage->at(attachData + parent - 1);
    return attachData + (paletteNode >> 8) + Utils::bitCount8(paletteNode & 0xFF & ((1 << childIndex) - 1));
}

void Octree::select(uint32_t parent, uint32_t childIndex, bool append) {
    int address = colorAttachAddress(parent, childIndex);

    QSharedPointer<Node> node;
    node.reset(new Node);
    node->parent = parent;
    node->childIndex = childIndex;
    QColor color;

    int index = -1;
    for (int i = 0; i < m_selection.count(); i++) {
        if (m_selection.at(i)->parent == parent && m_selection.at(i)->childIndex == childIndex) {
            index = i;
            break;
        }
    }

    if (append) {
        if (index >= 0) { // Remove selection
            (*storage)[address] = m_selection.at(index)->color;
            m_selection.remove(index);
            nodeDeselected();
        } else { // Append selection
            node->color = (*storage)[address];
            (*storage)[address] = selectionColor;
            m_selection.append(node);
            color.setRgba(node->color);
            nodeSelected(childIndex, color);
        }
    } else if (index == -1 || m_selection.count() > 1) {
        deselect();
        node->color = (*storage)[address];
        (*storage)[address] = selectionColor;
        m_selection.append(node);
        color.setRgba(node->color);
        nodeSelected(childIndex, color);
    }

    dataChanged();
}

void Octree::changeNodeColor(const QColor& color) {
    for (auto node: m_selection) {
        node->color = color.rgba();
    }
    setIsModified(true);
}

void Octree::deselect() {
    if (m_selection.count()) {
        for (auto node: m_selection) {
            int address = colorAttachAddress(node->parent, node->childIndex);
            (*storage)[address] = node->color;
        }

        m_selection.clear();
        nodeDeselected();
        dataChanged();
    }
}

void Octree::splitNode() {
    qDebug() << "split";
}

void Octree::mergeNode() {
    qDebug() << "merge";
}

void Octree::addNode() {
    qDebug() << "add";
}

void Octree::deleteNode() {
    if (source.deleteNode(m_selection)) {
        m_selection.clear();
        nodeDeselected();
        storage = source.binary();
        setIsModified(true);
        dataChanged();
    }
}
