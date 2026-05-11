#include "KeyboardWidget.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

QString styleForKind(const QString& kind) {
    if (kind == "digit") {
        return "QPushButton {"
               "  background: #2d2d2d; color: white; border: 1px solid #555;"
               "  border-radius: 6px; padding: 8px; font-size: 14px;"
               "} QPushButton:hover { background: #3a3a3a; }"
               " QPushButton:pressed { background: #1f1f1f; }";
    }
    if (kind == "op") {
        return "QPushButton {"
               "  background: #2d3e50; color: white; border: 1px solid #4a6a8a;"
               "  border-radius: 6px; padding: 8px; font-size: 14px;"
               "} QPushButton:hover { background: #3a5070; }"
               " QPushButton:pressed { background: #1f2c3e; }";
    }
    if (kind == "func") {
        return "QPushButton {"
               "  background: #3d2e4d; color: white; border: 1px solid #6a4a8a;"
               "  border-radius: 6px; padding: 6px; font-size: 13px;"
               "} QPushButton:hover { background: #503a66; }"
               " QPushButton:pressed { background: #2e1f3e; }";
    }
    if (kind == "var") {
        return "QPushButton {"
               "  background: #2e4d3d; color: white; border: 1px solid #4a8a6a;"
               "  border-radius: 6px; padding: 8px; font-size: 14px; font-weight: bold;"
               "} QPushButton:hover { background: #3a6650; }"
               " QPushButton:pressed { background: #1f3e2e; }";
    }
    if (kind == "ctrl") {
        return "QPushButton {"
               "  background: #5a3a3a; color: white; border: 1px solid #8a5a5a;"
               "  border-radius: 6px; padding: 8px; font-size: 14px; font-weight: bold;"
               "} QPushButton:hover { background: #6e4a4a; }"
               " QPushButton:pressed { background: #3e2828; }";
    }
    if (kind == "build") {
        return "QPushButton {"
               "  background: #3a7a3a; color: white; border: 1px solid #5aaa5a;"
               "  border-radius: 6px; padding: 10px; font-size: 16px; font-weight: bold;"
               "} QPushButton:hover { background: #4a9a4a; }"
               " QPushButton:pressed { background: #2a5a2a; }";
    }
    return {};
}

}  // namespace

KeyboardWidget::KeyboardWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);
    root->setContentsMargins(6, 6, 6, 6);

    // ---- Functions group ------------------------------------------------
    auto* funcGroup = new QGroupBox(QStringLiteral("Функции"), this);
    auto* funcGrid = new QGridLayout(funcGroup);
    funcGrid->setSpacing(4);

    struct FuncDef { const char* label; const char* name; };
    const FuncDef funcs[] = {
        {"sin",  "sin"},  {"cos",  "cos"},  {"tan",  "tan"},
        {"arcsin","asin"},{"arccos","acos"},{"arctan","atan"},
        {"sinh", "sinh"}, {"cosh", "cosh"}, {"tanh", "tanh"},
        {"log",  "log"},  {"ln",   "ln"},   {"√",    "sqrt"},
        {"|x|",  "abs"},  {"exp",  "exp"},
    };
    const int kCols = 3;
    for (int i = 0; i < static_cast<int>(std::size(funcs)); ++i) {
        auto* btn = makeButton(QString::fromUtf8(funcs[i].label), "func");
        const QString name = QString::fromLatin1(funcs[i].name);
        connect(btn, &QPushButton::clicked, this, [this, name]() {
            emit functionRequested(name);
        });
        funcGrid->addWidget(btn, i / kCols, i % kCols);
    }
    root->addWidget(funcGroup);

    // ---- Digits + operators + control group -----------------------------
    auto* keysGroup = new QGroupBox(QStringLiteral("Ввод"), this);
    auto* grid = new QGridLayout(keysGroup);
    grid->setSpacing(4);

    auto addText = [&](const QString& label, const QString& sent, const QString& kind, int row, int col) {
        auto* btn = makeButton(label, kind);
        connect(btn, &QPushButton::clicked, this, [this, sent]() {
            emit textRequested(sent);
        });
        grid->addWidget(btn, row, col);
        return btn;
    };

    // Row 0: C, ⌫, (, )
    {
        auto* c = makeButton("C", "ctrl");
        connect(c, &QPushButton::clicked, this, &KeyboardWidget::clearRequested);
        grid->addWidget(c, 0, 0);

        auto* bs = makeButton(QString::fromUtf8("⌫"), "ctrl");
        connect(bs, &QPushButton::clicked, this, &KeyboardWidget::backspaceRequested);
        grid->addWidget(bs, 0, 1);

        addText("(", "(", "op", 0, 2);
        addText(")", ")", "op", 0, 3);
    }

    // Row 1: 7 8 9 +
    addText("7", "7", "digit", 1, 0);
    addText("8", "8", "digit", 1, 1);
    addText("9", "9", "digit", 1, 2);
    addText("+", "+", "op",    1, 3);

    // Row 2: 4 5 6 -
    addText("4", "4", "digit", 2, 0);
    addText("5", "5", "digit", 2, 1);
    addText("6", "6", "digit", 2, 2);
    addText("-", "-", "op",    2, 3);

    // Row 3: 1 2 3 *
    addText("1", "1", "digit", 3, 0);
    addText("2", "2", "digit", 3, 1);
    addText("3", "3", "digit", 3, 2);
    addText("*", "*", "op",    3, 3);

    // Row 4: 0 . x /
    addText("0", "0", "digit", 4, 0);
    addText(".", ".", "digit", 4, 1);
    addText("x", "x", "var",   4, 2);
    addText("/", "/", "op",    4, 3);

    // Row 5: π, e, ^, =
    addText(QString::fromUtf8("π"), "pi", "var", 5, 0);
    addText("e", "e", "var", 5, 1);
    addText("^", "^", "op",  5, 2);
    {
        auto* eq = makeButton("=", "build");
        connect(eq, &QPushButton::clicked, this, &KeyboardWidget::buildRequested);
        grid->addWidget(eq, 5, 3);
    }

    // Row 6: ← →
    {
        auto* lf = makeButton(QString::fromUtf8("←"), "ctrl");
        connect(lf, &QPushButton::clicked, this, [this]() { emit cursorMoveRequested(-1); });
        grid->addWidget(lf, 6, 0, 1, 2);

        auto* rt = makeButton(QString::fromUtf8("→"), "ctrl");
        connect(rt, &QPushButton::clicked, this, [this]() { emit cursorMoveRequested(+1); });
        grid->addWidget(rt, 6, 2, 1, 2);
    }

    root->addWidget(keysGroup);

    // ---- Build (=) button (also accessible as a wide button below) -----
    auto* buildBtn = makeButton(QStringLiteral("= Построить график"), "build");
    connect(buildBtn, &QPushButton::clicked, this, &KeyboardWidget::buildRequested);
    root->addWidget(buildBtn);

    root->addStretch(1);
}

QPushButton* KeyboardWidget::makeButton(const QString& label, const QString& kind) {
    auto* btn = new QPushButton(label, this);
    btn->setMinimumSize(48, 36);
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setStyleSheet(styleForKind(kind));
    return btn;
}

void KeyboardWidget::addAt(QGridLayout* layout, QPushButton* btn, int row, int col, int rowSpan, int colSpan) {
    layout->addWidget(btn, row, col, rowSpan, colSpan);
}
