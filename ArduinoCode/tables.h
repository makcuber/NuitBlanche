/*
<<<File Info>>>
Description: Code to define a table that can be printed over serial communications

Developer: Jonathan Brunath

Date Created: 1/12/12/2016
*/

#include "Arduino.h"
#include "String.h"

//
//Table definitions
//
const int maxTableCells=5;

using namespace std;

class tableRow{
  private:
    String cells[maxTableCells];
    int rowWidth;
  public:
    tableRow(){
      rowWidth=maxTableCells;
    }
    tableRow(int width){
      RowWidth(width);
      for(int i=0;i<rowWidth;i++){
        cells[i]="";
      }
    }
    void RowWidth(int width){
      if((width>0)&&(width<maxTableCells)){
        rowWidth=width;
      }else{
        rowWidth=maxTableCells;
      }
    }
    int RowWidth(){
      return rowWidth;
    }
    void addData(String data){
      for(int i=0;i<rowWidth;i++){
        if(cells[i]==""){
          cells[i]=data;
        }
      }
    }
    void addData(String data, int position){
      if((position>0)&&(position<rowWidth)){
        cells[position]=data;
      }else{
        cells[rowWidth]=data;
      }
    }
    void print(){
      if(Serial){
        for(int i=0;i<rowWidth;i++){
          Serial.print("|\t"+cells[i]);
        }
        Serial.print("|");
      }
    }
};
class tableObj{
  public:
    tableRow *titles;
    tableObj(int width){
      titles=new tableRow(width);
    }
    void addRow(String s0){
      addRow(s0, "", "", "", "");
    }
    void addRow(String s0,String s1){
      addRow(s0, s1, "", "", "");
    }
    void addRow(String s0,String s1,String s2){
      addRow(s0, s1, s2, "", "");
    }
    void addRow(String s0,String s1,String s2,String s3){
      addRow(s0, s1, s2, s3, "");
    }
    void addRow(String s0,String s1,String s2,String s3,String s4){
      String tmp[]={s0,s1,s2,s3,s4};
      tableRow row;
      for(int i=0;i<maxTableCells;i++){
        if(tmp[i]!=""){
          row.addData(tmp[i]);
        }
      }
      addRow(&row);
    }
    void addRow(tableRow *row){
      row->RowWidth(titles->RowWidth());
      row->print();
    }
};
