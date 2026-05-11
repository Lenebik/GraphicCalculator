#include "InputField.h"

#include <QFont>
#include <QKeyEvent>

InputField::InputField(QWidget* parent) : QLineEdit(parent) {
    setMaxLength(256);
    setPlaceholderText(QStringLiteral("Введите функцию от x, например: sin(x) + 2x"));
    setStyleSheet(
        "QLineEdit {"
        "  background: #1e1e1e;"
        "  color: #ffffff;"
        "  border: 1px solid #555;"
        "  border-radius: 6px;"
        "  padding: 8px 10px;"
        "  selection-background-color: #2a6fb0;"
        "}");
    QFont f = font();
    f.setPointSize(f.pointSize() + 4);
    f.setFamily("Menlo");
    setFont(f);
    setCursor(Qt::IBeamCursor);
    setContextMenuPolicy(Qt::NoContextMenu);
    setCursorMoveStyle(Qt::LogicalMoveStyle);
}

void InputField::setMaxLength(int length) {
    QLineEdit::setMaxLength(length);
}

void InputField::insertAtCursor(const QString& text) {
    int pos = cursorPosition();
    QString next = this->text();
    next.insert(pos, text);
    if (next.length() > maxLength()) return;
    setText(next);
    setCursorPosition(pos + text.length());
    setFocus();
}

void InputField::insertFunctionCall(const QString& name) {
    int pos = cursorPosition();
    QString insertion = name + "()";
    QString next = this->text();
    next.insert(pos, insertion);
    if (next.length() > maxLength()) return;
    setText(next);
    setCursorPosition(pos + name.length() + 1);
    setFocus();
}

void InputField::backspaceAtCursor() {
    int pos = cursorPosition();
    if (pos <= 0) return;
    QString next = text();
    next.remove(pos - 1, 1);
    setText(next);
    setCursorPosition(pos - 1);
    setFocus();
}

void InputField::keyPressEvent(QKeyEvent* event) {
    // Physical keyboard is blocked except for the explicitly listed control keys.
    switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit buildRequested();
            event->accept();
            return;
        case Qt::Key_Escape:
            emit clearRequested();
            event->accept();
            return;
        case Qt::Key_Backspace:
            backspaceAtCursor();
            event->accept();
            return;
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Home:
        case Qt::Key_End:
            QLineEdit::keyPressEvent(event);
            return;
        default:
            // Swallow every other key (including text input) — only the soft keyboard
            // is allowed to mutate the field.
            event->accept();
            return;
    }
}
