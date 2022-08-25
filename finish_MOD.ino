

//監控警示燈(GREEN 2.2v)
#define  monitor_LED_G  10
//監控警示燈(RED 1.8v)
#define  monitor_LED_R  11
//是否監控有無變化
#define  monitor_DI2  4
//H Alarm 輸入
#define  o2_alarm_1_DI  7
//HH Alarm輸入
#define  o2_alarm_2_DI  8
//輸出alarm
#define  o2_alarm_DO  9
//警報時interlock
#define  interlock_DO  12
//AI輸入訊號轉電壓參數 5V/1024=0.005
#define AI_Voltage_parameter 0.005
//電壓轉ppm參數 5V-1000ppm arduino 輸入為0-5V
#define Voltage_ppm_parameter 200
//取幾筆
#define record_leng 60;
int O2_singal_AI = A0;
//===============狀態==============================
bool statue_control_1 = false;//紀錄是否開啟5%監測狀態
//================================================
int  statue_LED_1 = 0;//濃度狀態燈號
//====================================================
int count1 = 0;//計數器1
//暫存第一次測值

//暫存目前測值
double measured_now = 0.0;
//alarm狀態
int alarm_state = 0;
//誤差百分比設定值:<10%
int diff_value_percent_alarm = 16;
//誤差百分比 
double diff_value_percent = 0.0;

double diff_value = 0.0;//誤差值

bool o2_alarm_flage = false;//百分比濃度誤差警報
// mv
double real_mv_1 = 0.0;

int alarm_delay_1 = 0;//切換延遲時間，每次切換時氣體閉鎖濃度飆高，等待5秒穩定=循環5次計數
int timer_H_alarm = 5;//alarm後延遲秒數警報

const int a_size = 10;// 陣列大小
//暫存AI訊號
int temp_AI[a_size];//儲存數量越小越能及時反應
//一循環秒數(ms)
const int delay_timer = 1000;
//間距多久時間檢查濃度誤差
const int mearure_check = 180;//3分鐘
void quickSort(int s[], int l, int r);
//眾數暫存器
int Mode_ai = 0;
//============filter================
//過濾後AI
int filter_AI = 0;//o2輸入訊號
double f_Singal_convert_Voltage = 0.0;//訊號轉換電壓值(電壓值)
double f_mv_1 = 0.0;
double f_o2_sensor_ppm = 0.0; //o2測值(轉換成ppm)
//============filter================
// ............初始值............
double measured_value1 = 0.0;//暫存第一筆值
// ............初始值............
//---------即時值---------------------
int real_o2_sensor_AI = 0;//即時o2輸入訊號
double real_o2_sensor_ppm = 0.0;//即時o2測值(轉換成ppm)
double real_Singal_convert_Voltage = 0.0;//訊號轉換電壓值(即時電壓值)
//---------即時值---------------------

void setup() {

    pinMode(O2_singal_AI, INPUT);

    pinMode(monitor_DI2, INPUT);
    pinMode(monitor_LED_R, OUTPUT);
    pinMode(monitor_LED_G, OUTPUT);
    pinMode(o2_alarm_1_DI, INPUT);
    pinMode(o2_alarm_2_DI, INPUT);
    pinMode(o2_alarm_DO, OUTPUT);
    pinMode(interlock_DO, OUTPUT);
    Serial.begin(9600);
}

void loop() {

    real_o2_sensor_AI = analogRead(O2_singal_AI);
    /// <summary>
    /// //先將陣列填滿，否則眾數為0
    /// </summary>
    if (count1 <= 1)//從頭開始時需讓陣列有值，否則為0
    {
        fill_tempAI_array();
    }
    if (abs(real_o2_sensor_AI - filter_AI) > 10)//AI輸入即時值和過濾AI值差距>15，直接以即時值取代過濾AI值，變化速度才能即時反應
    {
        fill_tempAI_array();
    }
    if (count1 > a_size)
    {
        temp_AI[count1 % a_size] = real_o2_sensor_AI;//超過陣列大小重頭開始覆蓋寫入
    }
    else
    {
        temp_AI[count1] = real_o2_sensor_AI;
    }

    {
        Serial.print("即時值_AI: ");
        Serial.println(real_o2_sensor_AI);

    }

    {
        /*for (int i = 0; i < 30; i++) {
          Serial.print("numd[");
          Serial.print(i);
          Serial.print("]: ");
          Serial.println(temp_AI[i]);
          Serial.print(",");
          Serial.print("");
          }Serial.println("");*/
    }

    /*for (int i = 0; i < 30; i++) {
      Serial.print("numd[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.println(numd[i]);
      Serial.print(",");
      Serial.print("");
    }Serial.println("");*/
    //counting_sort(temp_AI, 29);//取出眾數(過濾雜訊)
      //Mode_ai = ;

    filter_measure();


    if (count1 != 0)
    {
        //v1 = (v1 + temp_o2_sensor_ppm) / count1;
    }

    if (digitalRead(o2_alarm_1_DI) == HIGH || digitalRead(o2_alarm_2_DI) == HIGH)
    {

        alarm_delay_1++;

        if (alarm_delay_1 >= timer_H_alarm)//5秒=5次循環後濃度仍然H，則alarm
        {
            alarm_state = 1;
        }
    }
    else if (o2_alarm_flage == false)
    {
        alarm_delay_1 = 0;
        alarm_state = 0;
    }

    //============5%監測======================
    /*Serial.print("======================================statue_control_1: ");
    Serial.println(statue_control_1);
    Serial.print("======================================monitor_DI2: ");
    Serial.println(monitor_DI2);*/
    if (digitalRead(monitor_DI2) == 0)//沒開監測
    {
        o2_alarm_flage = false;
        statue_control_1 = false;
    }
    if (statue_control_1 == false && digitalRead(monitor_DI2) == 1)//途中才開啟
    {

        Serial.println("======================================途中開啟監測: ");
        
        monitor_statue(); 
        Serial.println("開始監測: ");
        Serial.println(count1);
        count1 = 0;//從開啟5%監測開始，計數器重新開始計數
        statue_control_1 = true;
    }
    if (statue_control_1 == true)//開啟監測
    {
        if (count1 == 5)//起始時暫存當時濃度
        {
            real_measure();
            first_record_value();
        }
        Serial.print("紀錄值ppm: ");
        Serial.print(measured_value1);
        Serial.println(" ppm");
        if (count1 == mearure_check)
        {
            Serial.println("***************開始比較:********* ");

            quickSort(temp_AI, 0, a_size);//快速排序
            real_measure();
            diff_measure_value();
            Serial.print("紀錄值ppm: ");
            Serial.print(measured_value1);
            Serial.println(" ppm");
            Serial.print("實際值ppm: ");
            Serial.print(measured_now);
            Serial.println(" ppm");

            Serial.print("誤差%: ");
            Serial.print(diff_value_percent);
            Serial.println(" %");
            if (diff_value_percent <= diff_value_percent_alarm)
            {
                o2_alarm_flage = true;
                alarm_state = 2;
            }
            else
            {
                o2_alarm_flage = false;
            }
            //重新計數
            count1 = 0;

        }
    }
    //================================================


    Serial.print("count1: ");
    Serial.println(count1);
    count1++;
    //=====================統整狀態==================
    {
        if (alarm_state == 0)
        {
            Serial.println("statue_LED_1 = 0");
            statue_LED_1 = 0;
        }
        if (alarm_state == 1)
        {
            Serial.println("statue_LED_1 = 1");
            statue_LED_1 = 1;
        }
        if (alarm_state == 2)
        {
            Serial.println("statue_LED_1 = 2");
            statue_LED_1 = 2;
        }

        Serial.println("alarm_state: ");
        Serial.println(alarm_state);
        Serial.println("statue_LED_1: ");
        Serial.println(statue_LED_1);
        choose_alarm_state();
    }


    //================================================
    delay(delay_timer);
}
void O2_alarm()
{
    digitalWrite(o2_alarm_DO, HIGH);
    digitalWrite(interlock_DO, HIGH);
}
void O2_normal()
{
    digitalWrite(o2_alarm_DO, LOW);
    digitalWrite(interlock_DO, LOW);
}
void  fill_tempAI_array()
{
    for (int i = 0; i < a_size / 2 + 1; i++)//只需要填滿1/2+1陣列，比一半陣列多就可比較出MAX
    {
        temp_AI[i] = real_o2_sensor_AI;
    }
}
void quickSort(int arr[], int start, int length) {
    if (start < length)
    {
        int i = start, j = length; float x = arr[start];
        while (i < j)
        {
            while (i < j && arr[j] >= x) // 从右向左找第一个小于x的数
                j--;
            if (i < j)
                arr[i++] = arr[j];
            while (i < j && arr[i] < x) // 从左向右找第一个大于等于x的数
                i++;
            if (i < j)
                arr[j--] = arr[i];
        }
        arr[i] = x;
        quickSort(arr, start, i - 1); // 递归调用
        quickSort(arr, i + 1, length);
    }
}
int counting_sort(int arr[], int len)
{ //清空陣列

    int c[500] = { 0 };//0-500ppm
    Mode_ai = 0;
    // 統計數字數量
    for (int i = 0; i <= len; i++)
    {
        c[arr[i]]++;
    }
    quickSort(arr, 0, a_size);
    /* for (int i = 0; i <= len; i++)
     {
       Serial.print("數值:  ");
       Serial.print("次數: ");
       Serial.print(arr[i]);
       Serial.print(" ");
       Serial.println(c[arr[i]]);
     }*/
     //找出出現最多次  數值
    int max = c[arr[0]];
    int max_value = 0;
    for (int i = 1; i < len; i++)
    {
        if (c[arr[i]] >= max)//要=時也要成立，否則會變0
        {
            max = c[arr[i]];
            max_value = arr[i];
        }
    }
    Mode_ai = max_value;
    Serial.print("眾數數值: ");//數值
    Serial.println(max_value);
    Serial.print("MAX次數:  ");//找出出現最多次
    Serial.println(max);
    Serial.print(" ");
    return Mode_ai;
}
//實際測值
void real_measure()
{
    real_Singal_convert_Voltage = real_o2_sensor_AI * AI_Voltage_parameter;
    real_mv_1 = real_Singal_convert_Voltage * 1000;
    real_o2_sensor_ppm = real_Singal_convert_Voltage * Voltage_ppm_parameter;
}
void filter_measure()
{
    filter_AI = counting_sort(temp_AI, a_size);
    f_Singal_convert_Voltage = filter_AI * AI_Voltage_parameter;
    f_mv_1 = f_Singal_convert_Voltage * 1000;
    f_o2_sensor_ppm = f_Singal_convert_Voltage * Voltage_ppm_parameter;
}
//計算誤差值
void diff_measure_value()
{
    measured_now = real_o2_sensor_ppm;//當下及時測值
    diff_value = abs((measured_value1 - measured_now) / measured_now);
    diff_value_percent = diff_value * 100;

}
//暫存第一筆測值
void first_record_value()
{
    measured_value1 = real_o2_sensor_ppm;
    Serial.print("---------------開始記錄ppm: ");
    Serial.print(measured_value1);
    Serial.println(" ppm");
}
//選擇狀態
void choose_alarm_state()
{
    Serial.println("statue_LED_1: ");
    Serial.println(statue_LED_1);
    switch (statue_LED_1)
    {
    case 0: //正常狀態
        monitor_statue();
        O2_normal();
        Serial.println("O2_normal()");
        break;
    case 1: //濃度異常
        monitor_statue();
        O2_alarm();
        Serial.println("O2_alarm()，實際濃度高於設定值 ");
        break;
    case 2://誤差值<5%
        O2_alarm();
        analogWrite(monitor_LED_G, 0);
        analogWrite(monitor_LED_R, 91);
        Serial.println("O2_alarm()，誤差值<5% ");
        break;
    }
}
void monitor_statue()
{
    if (statue_control_1 == true)
    {
        analogWrite(monitor_LED_G, 116);
        analogWrite(monitor_LED_R, 0);
    }
    else
    {
        analogWrite(monitor_LED_G, 0);
        analogWrite(monitor_LED_R, 0);
    }


}
