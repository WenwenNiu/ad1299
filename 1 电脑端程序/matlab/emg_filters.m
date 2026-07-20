classdef emg_filters<handle

    properties (Constant)   % 定义类常量

    end

    properties   %定义属性－－－类变量
        hds_500 = [];
        zf_500 = [];
        hds_1000 = [];
        zf_1000 = [];
        hds_2000 = [];
        zf_2000 = [];
    end

    methods

        function obj = emg_filters()
            obj.hds_500 =[emg_hp_f(500,20) emg_comb_f(500)];
            obj.zf_500 = cell(length(obj.hds_500),1);

            obj.hds_1000 =[emg_hp_f(1000,20) emg_comb_f(1000)];
            obj.zf_1000 = cell(length(obj.hds_1000),1);

            obj.hds_2000 =[emg_hp_f(2000,20) emg_comb_f(2000)];
            obj.zf_2000 = cell(length(obj.hds_2000),1);
        end

        function [data,zfs] = hds_filter(obj,hds,zfs,data)
            for i = 1:length(hds)
                a = hds(i).Denominator;
                b = hds(i).Numerator;
                zf = cell2mat(zfs{i});
                [data,zf] = filter(b,a,data,zf);
                zfs{i} = {zf};
            end
        end

        function data = data_filter(obj,fs,data)

            switch(fs)
                case 500
                    [data,obj.zf_500] = hds_filter(obj,obj.hds_500,obj.zf_500,data);
                case 1000
                    [data,obj.zf_1000] = hds_filter(obj,obj.hds_1000,obj.zf_1000,data);
                case 2000
                    [data,obj.zf_2000] = hds_filter(obj,obj.hds_2000,obj.zf_2000,data);
            end
        end
    end
end

