classdef eeg_filters<handle

    properties (Constant)   % 定义类常量

    end

    properties   %定义属性－－－类变量
        hds_250 = [];
        zf_250 = [];
        hds_500 = [];
        zf_500 = [];
        hds_1000 = [];
        zf_1000 = [];
        hds_2000 = [];
        zf_2000 = [];
    end

    methods
        function obj = eeg_filters()

            obj.hds_250 =[eeg_hp_f(250,0.5) eeg_lp_f(250,100) eeg_noth_f(250,50)];
            obj.zf_250 = cell(length(obj.hds_250),1);

            obj.hds_500 =[eeg_hp_f(500,0.5) eeg_lp_f(500,100) eeg_noth_f(500,50) eeg_noth_f(500,100) ];
            obj.zf_500 = cell(length(obj.hds_500),1);

            obj.hds_1000 =[eeg_hp_f(1000,0.5) eeg_lp_f(1000,100) eeg_noth_f(1000,50) eeg_noth_f(1000,100)];
            obj.zf_1000 = cell(length(obj.hds_1000),1);

            obj.hds_2000 =[eeg_hp_f(2000,0.5) eeg_lp_f(2000,100) eeg_noth_f(2000,50) eeg_noth_f(2000,100)];
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
                case 250
                    [data,obj.zf_250] = hds_filter(obj,obj.hds_250,obj.zf_250,data);
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

