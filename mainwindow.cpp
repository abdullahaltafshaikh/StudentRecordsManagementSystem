#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

// ---------------------------------------------------------------
// NOTE: widget names below must match your Object Names in the .ui
//   namelineedit, fatherlineedit, castelineedit,
//   rolllineedit, districtlineedit, gpalineedit
//   addbutton, updatebutton, deletebutton, nextbutton
// ---------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // The text file is saved next to the .exe
    filePath = QCoreApplication::applicationDirPath() + "/students.txt";

    loadFromFile();
    setAddMode();   // Start: Add enabled, Update/Delete disabled
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ======================= FILE HANDLING =========================

// Reads every line: name,father,caste,roll,district,gpa
void MainWindow::loadFromFile()
{
    records.clear();

    QFile file(filePath);
    if (!file.exists())
        return; // first run, no file yet

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File Error", "Could not open " + filePath);
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        QStringList f = line.split(',');
        if (f.size() != 6)
            continue; // skip damaged lines

        Student s;
        s.name     = f[0];
        s.father   = f[1];
        s.caste    = f[2];
        s.roll     = f[3];
        s.district = f[4];
        s.gpa      = f[5].toDouble();
        records.append(s);
    }
    file.close();
}

// Rewrites the whole file from memory (used after add/update/delete)
bool MainWindow::saveToFile()
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, "File Error", "Could not write to " + filePath);
        return false;
    }

    QTextStream out(&file);
    for (const Student &s : records) {
        out << s.name << ',' << s.father << ',' << s.caste << ','
            << s.roll << ',' << s.district << ','
            << QString::number(s.gpa, 'f', 2) << '\n';
    }
    file.close();
    return true;
}

// ======================= FORM HELPERS ==========================

// Validates the form. Returns false (after showing a message) if invalid.
bool MainWindow::readForm(Student &s)
{
    s.name     = ui->nameLineEdit->text().trimmed();
    s.father   = ui->fatherLineEdit->text().trimmed();
    s.caste    = ui->casteLineEdit->text().trimmed();
    s.roll     = ui->rollLineEdit->text().trimmed();
    s.district = ui->districtLineEdit->text().trimmed();
    QString gpaText = ui->gpaLineEdit->text().trimmed();

    if (s.name.isEmpty() || s.father.isEmpty() || s.caste.isEmpty() ||
        s.roll.isEmpty() || s.district.isEmpty() || gpaText.isEmpty()) {
        QMessageBox::warning(this, "Missing Data", "Please fill in all fields.");
        return false;
    }

    // Commas would break the comma-separated file format
    const QStringList all = {s.name, s.father, s.caste, s.roll, s.district, gpaText};
    for (const QString &t : all) {
        if (t.contains(',')) {
            QMessageBox::warning(this, "Invalid Input", "Commas (,) are not allowed in any field.");
            return false;
        }
    }

    bool ok = false;
    s.gpa = gpaText.toDouble(&ok);
    if (!ok || s.gpa < 0.0 || s.gpa > 4.0) {
        QMessageBox::warning(this, "Invalid GPA", "GPA must be a number between 0.0 and 4.0.");
        return false;
    }
    return true;
}

void MainWindow::showRecord(const Student &s)
{
    ui->nameLineEdit->setText(s.name);
    ui->fatherLineEdit->setText(s.father);
    ui->casteLineEdit->setText(s.caste);
    ui->rollLineEdit->setText(s.roll);
    ui->districtLineEdit->setText(s.district);
    ui->gpaLineEdit->setText(QString::number(s.gpa, 'f', 2));
}

void MainWindow::clearForm()
{
    ui->nameLineEdit->clear();
    ui->fatherLineEdit->clear();
    ui->casteLineEdit->clear();
    ui->rollLineEdit->clear();
    ui->districtLineEdit->clear();
    ui->gpaLineEdit->clear();
}

// ========================= UI MODES ============================

// Add mode: blank form, only Add is enabled
void MainWindow::setAddMode()
{
    currentIndex = -1;
    clearForm();
    ui->rollLineEdit->setReadOnly(false);
    ui->addButton->setEnabled(true);
    ui->updateButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
    ui->nameLineEdit->setFocus();
}

// Browse mode: showing a saved record, Add disabled, Update/Delete enabled
void MainWindow::setBrowseMode(int index)
{
    currentIndex = index;
    showRecord(records[index]);
    ui->rollLineEdit->setReadOnly(true);  // Roll No is the unique ID, so it can't be edited
    ui->addButton->setEnabled(false);
    ui->updateButton->setEnabled(true);
    ui->deleteButton->setEnabled(true);
}

// ======================= BUTTON SLOTS ==========================

void MainWindow::on_addButton_clicked()
{
    Student s;
    if (!readForm(s))
        return;

    // Roll number must be unique
    for (const Student &r : records) {
        if (r.roll == s.roll) {
            QMessageBox::warning(this, "Duplicate", "A student with this Roll Number already exists.");
            return;
        }
    }

    records.append(s);
    if (saveToFile()) {
        QMessageBox::information(this, "Success", "Student record added successfully.");
        setAddMode();
    } else {
        records.removeLast(); // undo if saving failed
    }
}

void MainWindow::on_nextButton_clicked()
{
    if (records.isEmpty()) {
        QMessageBox::information(this, "No Records", "There are no saved records yet.");
        return;
    }

    int next = currentIndex + 1;
    if (next >= records.size()) {
        // Reached the end: go back to a blank form in Add mode
        setAddMode();
        return;
    }
    setBrowseMode(next);
}

void MainWindow::on_updateButton_clicked()
{
    if (currentIndex < 0 || currentIndex >= records.size())
        return;

    Student s;
    if (!readForm(s))
        return;

    // Roll number is read-only, so it still identifies the same record
    records[currentIndex] = s;
    if (saveToFile())
        QMessageBox::information(this, "Success", "Student record updated successfully.");
}

void MainWindow::on_deleteButton_clicked()
{
    if (currentIndex < 0 || currentIndex >= records.size())
        return;

    auto answer = QMessageBox::question(this, "Confirm Delete",
                                        "Delete record of " + records[currentIndex].name + "?");
    if (answer != QMessageBox::Yes)
        return;

    records.removeAt(currentIndex);
    if (saveToFile())
        QMessageBox::information(this, "Deleted", "Student record deleted.");

    setAddMode();
}