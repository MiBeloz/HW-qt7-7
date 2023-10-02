#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pb_clearResult->setCheckable(true);

    //Код ДЗ
    pGraphicForm = new GraphicForm(this);
    pChart = new QChart();
    pChart->legend()->setVisible(false);
    pChartView = new QChartView(pChart);
    pGraphClass = new GraphicChart(1);
    pGridLayout = new QGridLayout;
    pGraphicForm->setLayout(pGridLayout);
    pGridLayout->addWidget(pChartView);
    pChartView->show();
    pChartView->hide();

    //Этот код сделан только ради тренировки и усвоения материала
    connect(&ftrWtReadFile, &QFutureWatcher<QVector<uint32_t>>::finished, this, [&]{
        readData = ftrReadFile.result();
        ftrProcessFile = QtConcurrent::run([&]{ return ProcessFile(readData);});
        ftrWtProcessFile.setFuture(ftrProcessFile);
    });
    connect(&ftrWtProcessFile, &QFutureWatcher<QVector<double>>::finished, this, [&]{
        procesData = ftrProcessFile.result();
        ftrFindMax = QtConcurrent::run([&]{ return FindMax(procesData);});
        ftrWtFindMax.setFuture(ftrFindMax);
    });
    connect(&ftrWtFindMax, &QFutureWatcher<QVector<double>>::finished, this, [&]{
        maxs = ftrFindMax.result();
        ftrFindMin = QtConcurrent::run([&]{ return FindMin(procesData);});
        ftrWtFindMin.setFuture(ftrFindMin);
    });
    connect(&ftrWtFindMin, &QFutureWatcher<QVector<double>>::finished, this, [&]{
        mins = ftrFindMin.result();
        DisplayResult(mins, maxs);
    });
}

MainWindow::~MainWindow()
{
    delete ui;

    //Код ДЗ
    delete pGraphicForm;
    delete pChart;
    delete pChartView;
    delete pGridLayout;
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
    //auto findMax = [&](QVector<double> resultData){
//    QVector<double> x;
//    QVector<double> y;

//    double steps = 1000;
//    x.resize(steps);
//    x[0] = 0;
//    for(int i = 1; i < steps; i++){
//        x[i] = x[i-1] + 0.1;
//    }

//    int size = 1000;
//    if (size < resultData.size()) size == resultData.size();
//    y.resize(size);
//    for (int i = 0; i < size; ++i){
//        y[i] = resultData[i];
//    }

//    pGraphClass->AddDataToGrahp(x, y, 1000);

//    pGraphicForm->show();

//    pGraphClass->UpdateGraph(pChart);
//    pChartView->chart()->createDefaultAxes();
//    pChartView->show( );
    //};

        QVector<double> x;
        QVector<double> y;
        double step = 0.1;

        double minVal = 0;
        double maxVal = 1000 + step;

        double steps = round(((maxVal-minVal)/step));
        x.resize(steps);
        x[0] = minVal;
        for(int i = 1; i < steps; i++){
            x[i] = x[i-1]+step;
        }

        y.resize(steps);
        for (int i = 0; i < steps; ++i){
            y[i] = resultData[i];
        }

        pGraphClass->AddDataToGrahp(x, y, FIRST_GRAPH);

        pGraphClass->UpdateGraph(pChart);
        pChartView->chart()->createDefaultAxes();
        pChartView->show();




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


//    auto read = [&]{ return ReadFile(pathToFile, numberSelectChannel); };
//    auto process = [&](QVector<uint32_t> res){ return ProcessFile(res);};
//    auto findMax = [&](QVector<double> res){
//                                                maxs = FindMax(res);
//                                                mins = FindMin(res);
//                                                DisplayResult(mins, maxs);

//                                                /*
//                                                 * Тут необходимо реализовать код наполнения серии
//                                                 * и вызов сигнала для отображения графика
//                                                 */

//                                             };

//    auto result = QtConcurrent::run(read)
//                               .then(process)
//                               .then(findMax);


    //Этот код сделан только ради тренировки и усвоения материала
    ftrReadFile = QtConcurrent::run([&]{ return ReadFile(pathToFile, numberSelectChannel); });
    ftrWtReadFile.setFuture(ftrReadFile);
}


