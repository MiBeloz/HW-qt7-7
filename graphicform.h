#ifndef GRAPHICFORM_H
#define GRAPHICFORM_H

#include <QWidget>
#include <QGridLayout>

namespace Ui {
class GraphicForm;
}

class GraphicForm : public QWidget
{
    Q_OBJECT

public:
    explicit GraphicForm(QWidget *parent = nullptr);
    ~GraphicForm();

    void setLayout(QGridLayout *gridLayout);

private:
    Ui::GraphicForm *ui;
};

#endif // GRAPHICFORM_H
