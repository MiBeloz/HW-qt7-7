#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pb_clearResult->setCheckable(true);

    pGridLayout = new QGridLayout(this);
    pChart = new QChart;
    pChartView = new QChartView(pChart);
    pChart->legend()->setVisible(false);
    pChartView->setRenderHint(QPainter::Antialiasing);

    ui->chB_graphicShow->setChecked(true);
    connect(ui->chB_graphicShow, &QCheckBox::clicked, this, [&]{
        if (!ui->chB_graphicShow->isChecked()) {
           ui->lb_time->setEnabled(false);
           ui->lb_timeStart->setEnabled(false);
           ui->lb_timeEnd->setEnabled(false);
           ui->spB_timeStart->setEnabled(false);
           ui->spB_timeEnd->setEnabled(false);
           ui->pb_graphicShow->setEnabled(false);
        }
        else {
            ui->lb_time->setEnabled(true);
            ui->lb_timeStart->setEnabled(true);
            ui->lb_timeEnd->setEnabled(true);
            ui->spB_timeStart->setEnabled(true);
            ui->spB_timeEnd->setEnabled(true);
            ui->pb_graphicShow->setEnabled(true);
        }
    });
    connect(ui->pb_graphicShow, &QPushButton::clicked, this, [&]{
        if (pChart->series().size()){
            ui->statusbar->setStyleSheet("color: default");
            ui->statusbar->clearMessage();
            pChartView->show();
        }
        else {
            ui->statusbar->setStyleSheet("color: red");
            ui->statusbar->showMessage("График еще не построен!");
        }
    });
    connect(ui->spB_timeStart, &QSpinBox::textChanged, this, [&]{
        if (ui->spB_timeStart->value() > ui->spB_timeEnd->value() - 100){
            ui->spB_timeStart->setValue(ui->spB_timeEnd->value() - 100);
            ui->statusbar->setStyleSheet("color: red");
            ui->statusbar->showMessage("Время начала должно быть меньше времени конца!");
        }
        else {
            ui->statusbar->setStyleSheet("color: default");
            ui->statusbar->clearMessage();
        }
        pChart->removeAllSeries();
    });
    connect(ui->spB_timeEnd, &QSpinBox::textChanged, this, [&]{
        if (ui->spB_timeEnd->value() < ui->spB_timeStart->value() + 100){
            ui->spB_timeEnd->setValue(ui->spB_timeStart->value() + 100);
            ui->statusbar->setStyleSheet("color: red");
            ui->statusbar->showMessage("Время конца должно быть больше времени начала!");
        }
        else {
            ui->statusbar->setStyleSheet("color: default");
            ui->statusbar->clearMessage();
        }
        pChart->removeAllSeries();
    });
    connect(this, &MainWindow::sig_GraphicReady, this, &MainWindow::Rcv_GraphicReady);


    //Этот код сделан только ради тренировки и усвоения материала
//    connect(&ftrWtReadFile, &QFutureWatcher<QVector<uint32_t>>::finished, this, [&]{
//        readData = ftrReadFile.result();
//        ftrProcessFile = QtConcurrent::run([&]{ return ProcessFile(readData);});
//        ftrWtProcessFile.setFuture(ftrProcessFile);
//    });
//    connect(&ftrWtProcessFile, &QFutureWatcher<QVector<double>>::finished, this, [&]{
//        procesData = ftrProcessFile.result();
//        ftrFindMax = QtConcurrent::run([&]{ return FindMax(procesData);});
//        ftrWtFindMax.setFuture(ftrFindMax);
//    });
//    connect(&ftrWtFindMax, &QFutureWatcher<QVector<double>>::finished, this, [&]{
//        maxs = ftrFindMax.result();
//        ftrFindMin = QtConcurrent::run([&]{ return FindMin(procesData);});
//        ftrWtFindMin.setFuture(ftrFindMin);
//    });
//    connect(&ftrWtFindMin, &QFutureWatcher<QVector<double>>::finished, this, [&]{
//        if (ui->chB_graphicShow->isChecked()) {
//            QVector<double> x;
//            QVector<double> y;
//            QLineSeries *series = new QLineSeries(this);
//            double step = 0.1;

//            double minVal = ui->spB_timeStart->value();
//            double maxVal = ui->spB_timeEnd->value() + step;

//            if (maxVal >= procesData.size() * step) {
//                maxVal = procesData.size() * step;
//                ui->spB_timeEnd->setValue(procesData.size() * step);
//            }
//            double steps = round(((maxVal - minVal) / step));

//            x.resize(steps);
//            y.resize(steps);
//            for(int i = 0; i < steps; i++){
//                if (i == 0) {
//                    x[i] = minVal;
//                }
//                else {
//                    x[i] = x[i - 1] + step;
//                }
//                y[i] = procesData[i + minVal];

//                series->append(x[i], y[i]);
//            }

//            if (pChart->series().size()) {
//                pChart->removeAllSeries();
//            }
//            pChart->addSeries(series);
//            emit sig_GraphicReady();
//        }

//        mins = ftrFindMin.result();
//        DisplayResult(mins, maxs);
//    });
}

MainWindow::~MainWindow()
{
    delete ui;

    delete pChart;
    delete pChartView;
}



/****************************************************/
/*!
@brief:	Метод считывает данные из файла
@param: path - путь к файлу
        numberChannel - какой канал АЦП считать
*/
/****************************************************/
QVector<uint32_t> MainWindow::ReadFile(QString path, uint8_t numberChannel)
{

    QFile file(path);
    file.open(QIODevice::ReadOnly);

    if(file.isOpen() == false){

        if(pathToFile.isEmpty()){
            QMessageBox mb;
            mb.setWindowTitle("Ошибка");
            mb.setText("Ошибка открытия фала");
            mb.exec();
        }
    }
    else{

        //продумать как выйти из функции
    }

    QDataStream dataStream;
    dataStream.setDevice(&file);
    dataStream.setByteOrder(QDataStream::LittleEndian);

    QVector<uint32_t> readData;
    readData.clear();
    uint32_t currentWorld = 0, sizeFrame = 0;

    while(dataStream.atEnd() == false){
        dataStream >> currentWorld;

        if(currentWorld == 0xFFFFFFFF){
            dataStream >> currentWorld;

            if(currentWorld < 0x80000000){
                dataStream >> sizeFrame;

                if(sizeFrame > 1500){
                    continue;
                }

                for(uint32_t i = 0; i < sizeFrame / sizeof(uint32_t); i++){
                    dataStream >> currentWorld;

                    if((currentWorld >> 24) == numberChannel){
                        readData.append(currentWorld);
                    }
                }
            }
        }
    }
    ui->chB_readSucces->setChecked(true);
    return readData;
}

QVector<double> MainWindow::ProcessFile(const QVector<uint32_t> dataFile)
{
    QVector<double> resultData;
    resultData.clear();

    foreach (int word, dataFile) {
        word &= 0x00FFFFFF;
        if(word > 0x800000){
            word -= 0x1000000;
        }

        double res = ((double)word / 6000000) * 10;
        resultData.append(res);
    }
    ui->chB_procFileSucces->setChecked(true);

    return resultData;
}

QVector<double> MainWindow::FindMax(QVector<double> resultData)
{
    double max = 0, sMax=0;
    //Поиск первого максиума
    foreach (double num, resultData){
        //QThread::usleep(1);
        if(num > max){
            max = num;
        }
    }

    //Поиск 2го максимума
    foreach (double num, resultData){
        //QThread::usleep(1);
        if(num > sMax && (qFuzzyCompare(num, max) == false)){
            sMax = num;
        }
    }

    QVector<double> maxs = {max, sMax};
    ui->chB_maxSucess->setChecked(true);
    return maxs;
}

QVector<double> MainWindow::FindMin(QVector<double> resultData)
{
    double min = 0, sMin = 0;
    QThread::sleep(1);
    //Поиск первого максиума
    foreach (double num, resultData){
        if(num < min){
            min = num;
        }
    }
    QThread::sleep(1);
    //Поиск 2го максимума
    foreach (double num, resultData){
        if(num < sMin && (qFuzzyCompare(num, min) == false)){
            sMin = num;
        }
    }

    QVector<double> mins = {min, sMin};
    ui->chB_minSucess->setChecked(true);
    return mins;
}

void MainWindow::DisplayResult(QVector<double> mins, QVector<double> maxs)
{
    ui->te_Result->append("Расчет закончен!");

    ui->te_Result->append("Первый минимум " + QString::number(mins.first()));
    ui->te_Result->append("Второй минимум " + QString::number(mins.at(1)));

    ui->te_Result->append("Первый максимум " + QString::number(maxs.first()));
    ui->te_Result->append("Второй максимум " + QString::number(maxs.at(1)));
}

/****************************************************/
/*!
@brief:	Обработчик клика на кнопку "Выбрать путь"
*/
/****************************************************/
void MainWindow::on_pb_path_clicked()
{
    pathToFile = "";
    ui->le_path->clear();

    pathToFile =  QFileDialog::getOpenFileName(this,
                                              tr("Открыть файл"), "/home/", tr("ADC Files (*.adc)"));

    ui->le_path->setText(pathToFile);
}

/****************************************************/
/*!
@brief:	Обработчик клика на кнопку "Старт"
*/
/****************************************************/
void MainWindow::on_pb_start_clicked()
{
    //проверка на то, что файл выбран
    if(pathToFile.isEmpty()){
        QMessageBox mb;
        mb.setWindowTitle("Ошибка");
        mb.setText("Выберите файл!");
        mb.exec();
        return;
    }

    ui->chB_maxSucess->setChecked(false);
    ui->chB_procFileSucces->setChecked(false);
    ui->chB_readSucces->setChecked(false);
    ui->chB_minSucess->setChecked(false);

    int selectIndex = ui->cmB_numCh->currentIndex();
    //Маски каналов
    if(selectIndex == 0){
        numberSelectChannel = 0xEA;
    }
    else if(selectIndex == 1){
        numberSelectChannel = 0xEF;
    }
    else if(selectIndex == 2){
        numberSelectChannel = 0xED;
    }

    auto read = [&]{ return ReadFile(pathToFile, numberSelectChannel); };
    auto process = [&](QVector<uint32_t> res){ return ProcessFile(res);};
    auto findMax = [&](QVector<double> res){
                                                maxs = FindMax(res);
                                                mins = FindMin(res);
                                                DisplayResult(mins, maxs);

                                                if (ui->chB_graphicShow->isChecked()) {
                                                    QVector<double> x;
                                                    QVector<double> y;
                                                    QLineSeries *series = new QLineSeries(this);
                                                    double step = 0.1;

                                                    double minVal = ui->spB_timeStart->value();
                                                    double maxVal = ui->spB_timeEnd->value() + step;

                                                    if (maxVal >= res.size() * step) {
                                                        maxVal = res.size() * step;
                                                        ui->spB_timeEnd->setValue(res.size() * step);
                                                    }
                                                    double steps = round(((maxVal - minVal) / step));

                                                    x.resize(steps);
                                                    y.resize(steps);
                                                    for(int i = 0; i < steps; i++){
                                                        if (i == 0) {
                                                            x[i] = minVal;
                                                        }
                                                        else {
                                                            x[i] = x[i - 1] + step;
                                                        }
                                                        y[i] = res[i + minVal];

                                                        series->append(x[i], y[i]);
                                                    }

                                                    if (pChart->series().size()) {
                                                        pChart->removeAllSeries();
                                                    }
                                                    pChart->addSeries(series);

                                                    emit sig_GraphicReady();
                                                }
    };

    auto result = QtConcurrent::run(read)
                               .then(process)
                               .then(this, findMax);


    //Этот код сделан только ради тренировки и усвоения материала
//    ftrReadFile = QtConcurrent::run([&]{ return ReadFile(pathToFile, numberSelectChannel); });
//    ftrWtReadFile.setFuture(ftrReadFile);
}

void MainWindow::Rcv_GraphicReady()
{
    ui->statusbar->setStyleSheet("color: default");
    ui->statusbar->clearMessage();

    pGridLayout->addWidget(pChartView);
    pChartView->chart()->createDefaultAxes();
    pChartView->resize(800, 400);
    pChartView->show();
}


