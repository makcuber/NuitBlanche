/*
<<<File Info>>>
Description: Code to define a table that can be printed over serial communications

Developer: Jonathan Brunath

Date Created: 1/12/12/2016
*/

#include "Arduino.h"

//
//Table definitions
//
const int maxTableCells = 5;

using namespace std;

class tableRow {
  private:
    String cells[maxTableCells];
    int rowWidth;
  public:
    tableRow() {
      rowWidth = maxTableCells;
    }
    tableRow(int width) {
      RowWidth(width);
      for (int i = 0; i < rowWidth; i++) {
        cells[i] = "";
      }
    }
    ~tableRow() {}
    void RowWidth(int width) {
      if ((width > 0) && (width < maxTableCells)) {
        rowWidth = width;
      } else {
        rowWidth = maxTableCells;
      }
    }
    int RowWidth() {
      return rowWidth;
    }
    void addData(String data) {
      for (int i = 0; i < rowWidth; i++) {
        if (cells[i] == "") {
          cells[i] = data;
        }
      }
    }
    void addData(String data, int position) {
      if ((position > 0) && (position < rowWidth)) {
        cells[position] = data;
      } else {
        cells[rowWidth] = data;
      }
    }
    void print() {
      if (Serial) {
        for (int i = 0; i < rowWidth; i++) {
          Serial.print("|\t" + String(cells[i]));
        }
        Serial.print("|\n");
      }
    }
};
class tableObj {
  public:
    tableRow *titles;
    tableObj(int width) {
      titles = new tableRow(width);
    }
    ~tableObj() {};
    void addRow(String c0) {
      addRow(c0, "", "", "", "");
    }
    void addRow(String c0, String c1) {
      addRow(c0, c1, "", "", "");
    }
    void addRow(String c0, String c1, String c2) {
      addRow(c0, c1, c2, "", "");
    }
    void addRow(String c0, String c1, String c2, String c3) {
      addRow(c0, c1, c2, c3, "");
    }
    void addRow(String c0, String c1, String c2, String c3, String c4) {
      String tmp[] = {c0, c1, c2, c3, c4};
      tableRow row;
      for (int i = 0; i < maxTableCells; i++) {
        if (tmp[i] != "") {
          row.addData(tmp[i]);
        }
      }
      addRow(&row);
      delete tmp;
      delete &row;
    }
    void addRow(tableRow *row) {
      row->RowWidth(titles->RowWidth());
      row->print();
    }
};
