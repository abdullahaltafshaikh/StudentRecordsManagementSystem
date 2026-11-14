#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// One student record (saved as one comma-separated line in the text file)
struct Student
{
    QString name;
    QString father;
    QString caste;
    QString roll;      // unique identifier
    QString district;
    double  gpa = 0.0;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Auto-connected by object name (addbutton, updatebutton, ...)
    void on_addButton_clicked();
    void on_updateButton_clicked();
    void on_deleteButton_clicked();
    void on_nextButton_clicked();

private:
    Ui::MainWindow *ui;

    QVector<Student> records;   // all records kept in memory
    int currentIndex = -1;      // -1 = Add mode, otherwise index being viewed
    QString filePath;           // students.txt location

    // File handling
    void loadFromFile();
    bool saveToFile();

    // Form helpers
    bool readForm(Student &s);          // validates + fills s from the form
    void showRecord(const Student &s);
    void clearForm();

    // UI modes
    void setAddMode();
    void setBrowseMode(int index);
};

#endif // MAINWINDOW_H