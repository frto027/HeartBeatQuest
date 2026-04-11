#pragma once

namespace HeartBeat{

enum DataSourceType{
    DS_RANDOM,
    DS_LAN,
    DS_BLE,
    DS_OSC,
    DS_HypeRate,
    DS_Pulsoid,
};


bool IsDatasourceAbleToRecord();

class DataSource{
public:
    const DataSourceType dataSourceType;

    DataSource(DataSourceType ty):dataSourceType(ty){ }

    virtual void LateStart(){};

    virtual bool GetData(int& heartbeat) = 0;
    virtual long long GetEnergy() { return 0; };
    virtual void Update() { };

    template<typename T>
    T * as(){
        return dynamic_cast<T*>(this);
    }

    static DataSource * getInstance();
};
    

};//namespace HeartBeat