#pragma once

#include <QString>
#include <QWidget>

class QPushButton;
class QGridLayout;

class KeyboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit KeyboardWidget(QWidget* parent = nullptr);

signals:
    void textRequested(const QString& text);
    void functionRequested(const QString& name);
    void cursorMoveRequested(int delta);
    void clearRequested();
    void backspaceRequested();
    void buildRequested();

private:
    QPushButton* makeButton(const QString& label, const QString& kind);
    void addAt(QGridLayout* layout, QPushButton* btn, int row, int col, int rowSpan = 1, int colSpan = 1);
};
