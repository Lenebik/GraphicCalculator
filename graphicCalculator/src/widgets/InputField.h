#pragma once

#include <QLineEdit>

class InputField : public QLineEdit {
    Q_OBJECT
public:
    explicit InputField(QWidget* parent = nullptr);

    void setMaxLength(int length);

    void insertAtCursor(const QString& text);
    void insertFunctionCall(const QString& name);
    void backspaceAtCursor();

signals:
    void buildRequested();
    void clearRequested();

protected:
    void keyPressEvent(QKeyEvent* event) override;
};
