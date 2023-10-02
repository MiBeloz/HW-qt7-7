#include "graphicform.h"
#include "ui_graphicform.h"

GraphicForm::GraphicForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GraphicForm)
{
    ui->setupUi(this);
}

GraphicForm::~GraphicForm()
{
    delete ui;
}

void GraphicForm::setLayout(QGridLayout *gridLayout)
{
    ui->wid_chart->setLayout(gridLayout);
}
