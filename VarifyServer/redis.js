const config_modules = require('./config')
const Redis = require("ioredis");

//创建Redis客户端实例
const RedisCli = new Redis({               //从 config.js读取config.json的服务器配置信息
    host: config_modules.redis_host,       //Redis服务器主机名
    port: config_modules.redis_port,       //Redis服务器端口号
    password: config_modules.redis_passwd, //Redis密码
    });

/**
* 监听错误信息，建立连接
 */
RedisCli.on("error",function (err) {          //连接失败则抛出错误信息，并退出RedisCli客户端
    console.log("RedisCli connect error");
    RedisCli.quie();
});


/**
 * 根据key获取value
 * @param {*} key
 * @returns
 */
async function GetRedis(key){
    try{
        const result = await RedisCli.get(key);  //这里用await和之前一样，因为Redis的Get返回式Promise，不保证返回结果。阻塞等待返回结果
        if (result == null){
            console.log('Result:' ,'<'+ result + '>','This key cannot be find...');
            return null;
        }
        console.log('Result:' ,'<' + result + '>','Get key success!...');
        return result;
    }
    catch(error){
        console.log('GetRedis error is:' , error);
        return null;
    }
}

/**
 * 根据key查询redis中是否存在key
 * @param {*} key
 * @returns
 */
async function QueryRedis(key){
    try{
        const result = await RedisCli.exists(key);  //这里用await和之前一样，因为Redis的Get返回式Promise，不保证返回结果。阻塞等待返回结果
        //判断该值是否为空，如果为空返回null
        if (result == 0){
            console.log('Result:' ,'<'+ result + '>','This key is null...');
            return null;
        }
        console.log('Result:' ,'<' + result + '>','with this value!...');
        return result;
    }
    catch(error){
        console.log('QueryRedis error is:' , error);
        return null;
    }
}

/**
 * 设置key和value，并设置过期时间
 * @param {*} key
 * @param {*} value
 * @param {*} exptime
 * @returns
 */
async function SetRedisExpire(key,value,exptime){
    try{
        //设置键和值
        await RedisCli.set(key,value);
        //设置过期时间（以秒为单位）
        await RedisCli.expire(key,value);
        return true;
    }
    catch(error){
        console.log('SetRedisExpire error is:' , error);
        return false;
    }
}

/**
 * 退出函数
 */
function Quit(){
    RedisCli.quit();
}

module.exports = {GetRedis,SetRedisExpire,QueryRedis,Quit}    //把这些接口抛出去让外部可以使用