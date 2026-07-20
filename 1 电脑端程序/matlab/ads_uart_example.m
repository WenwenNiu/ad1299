clc
clear
close all

% 绘图
adsPlot = ads_plot;

% 指令集
adsCmd = ads_cmd;

% 数据集
adsData = ads_data;

% 采样率，需与硬件保持一致，不然数字滤波器对应不上
adsData.rate = 500;

% 采集的信号类型,可选EMG、EEG,根据信号类型，选择相应的数字滤波器
adsData.signal_type = "EEG";

% 串口对象
uart = serialport('COM11',1000000);
configureCallback(uart,"off");
setRTS(uart,false)
setDTR(uart,false)
pause(0.5);  

disp('开始采集');
write(uart,adsCmd.startCollectCmd(),"uint8");

% 等待数据接收
while(adsPlot.isExitCollect == 0)
   
    pause(0.05);  

    % 读取串口数据并解析
    if uart.NumBytesAvailable > 0
        dataRev = read(uart,uart.NumBytesAvailable,"uint8");
        adsData.dataUnpack(dataRev);
    end

    % 更新绘图
    chx_data = adsData.chx_data;
    if ~isempty(chx_data)
       winLen = 2500;
       for ch =1:8
           adsPlot.plot_chx_data(ch,chx_data(:,ch) + ch*100,winLen);
       end
    end
end

disp('停止采集')
write(uart,adsCmd.stopCollectCmd(),"uint8");
pause(0.5)

% 关闭串口
clear uart

% 绘制曲线图
hold on
chx_val = adsData.chx_data;
if ~isempty(chx_val)
    for ch = 1:8
       plot(chx_val(:,ch) + ch*1000) 
    end
end
hold off





