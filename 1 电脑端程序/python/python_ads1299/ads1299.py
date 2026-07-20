import struct
import numpy as np
from fontTools.misc.cython import returns
from ads1299_filter import EmgFilter, EcgFilter


# 数据解析类
class ADS1298Data:
    def __init__(self):

        self.rx_buff = []   # 数据包接收缓存区
        self.emg_data = [[] for _ in range(8)]  # 初始化8个通道

        self.sample_rate = 500  # 采样率
        self.emg_filters = [EmgFilter() for _ in range(8)]
        self.emgFilter = EmgFilter()

    def clear(self):
        self.rx_buff = []
        self.emg_data = [[] for _ in range(8)]



    # 解包函数
    def parse_data(self, data_in):

        self.rx_buff += data_in
        data = (self.rx_buff)

        while len(data) > 6:

            checkByte = data[1] ^ data[2]

            # 帧头校验
            if (data[0] == 0xA5) and (data[3] == checkByte):

                # 帧长度
                framLen = data[1] + 3

                # 判断剩于数据长度是否大于帧长度
                if framLen <= len(data):

                    # 校验帧尾
                    if data[framLen - 1] == 0x5A:
                        self.frame_unpack(bytes(data[0:framLen]))
                        del data[0:framLen]
                    else:
                        data.pop(0)
                        print("校验帧尾出错!")
                else:
                    break
            else:
                data.pop(0)
                print("校验帧头出错!")

        self.rx_buff = data

    def frame_unpack(self, data):

        # 帧地址
        frameType = data[2]

        if frameType == ADS1298cmd.ADDRESS_SAMPLE_PAR:
            sample_rates = [125,250,500,1000,2000]
            self.sample_rate = sample_rates[data[4]]

        # 原始信号数据帧
        if frameType == ADS1298cmd.ADDRESS_START:
            data_num = int((data[1] - 2) / (4*8))
            temp = data[4:(4 + data_num * 32)]
            data_temp = struct.unpack(f"<{8 * data_num}f", temp)


            for i in range(data_num):
                # 获取同一时刻的8通道数据
                ch_data = data_temp[i * 8: i * 8 + 8]
                filtered_ch_data = []
                for ch in range(8):
                    # 使用通道专属滤波器
                    filtered = self.emg_filters[ch].process_buffer(self.sample_rate, [ch_data[ch]])[0]
                    filtered_ch_data.append(filtered)

                # 保存滤波后的数据
                for ch in range(8):
                    self.emg_data[ch].append(filtered_ch_data[ch])




# 指令集
class ADS1298cmd:
    ADDRESS_HW_VERSION = 0x01  #硬件版本
    ADDRESS_SOFTWARE = 0x02  #软件版本
    ADDRESS_DEVICE_NAME = 0x03  #设备名称
    ADDRESS_DEVICE_MAC = 0x04  # MAC地址
    ADDRESS_POWER = 0x05  #电量信息
    ADDRESS_RESET = 0x06 # 复位
    ADDRESS_SAMPLE_PAR = 0x10 # 采样参数
    ADDRESS_START = 0x11  #开始/停止采集指令

    @staticmethod
    def cmd_data_pack(addr, is_write, data):

        cmd = []

        # 帧头
        cmd.append(0xAA)
        # 帧长度
        cmd.append(len(data) + 3)
        if is_write:
            cmd.append(0x80)  # 写指令
        else:
            cmd.append(0x81)  # 读指令
        # 地址
        cmd.append(addr)

        # 数据内容
        cmd += data

        # 校验码（待定）
        cmd.append(0x00)

        # 帧尾
        cmd.append(0xBB)

        # 计算校验码

        xor = 0x00
        for b in cmd[1:-1]:
            xor ^= b
        cmd[len(cmd) - 2] = xor
        return cmd



    # 开始采集指令
    @staticmethod
    def start_collect_cmd():
        data = [0] * 9
        data[0] = 0x03
        return ADS1298cmd.cmd_data_pack(ADS1298cmd.ADDRESS_START, True, data)

    # 停止采集指令
    @staticmethod
    def stop_collect_cmd():
        data = [0] * 9
        data[0] = 0x00
        return ADS1298cmd.cmd_data_pack(ADS1298cmd.ADDRESS_START, True, data)


