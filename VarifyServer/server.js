const grpc = require('@grpc/grpc-js');
const message_proto = require('./proto');
const const_module = require('./const');
const { v4: uuidv4 } = require('uuid');
const emailModule = require('./email');

async function GetVarifyCode(call, callback) {     //async，支持异步的函数，后面要用await
    console.log("email is ", call.request.email)
    try{
        uniqueId = uuidv4();
        console.log("uniqueId is ", uniqueId)
        let text_str =  '您的验证码为'+ uniqueId +'请三分钟内完成注册'
        //发送邮件
        let mailOptions = {
            from: 'renyangyang18@163.com',
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };
    
        let send_res = await emailModule.SendMail(mailOptions);   //这里和email.js里的SendMail那里用Promise改成同步一样。await等待promise,意味着promise的回调触发
        console.log("send res is ", send_res)

        callback(null, { email:  call.request.email,
            error:const_module.Errors.Success
        }); 
        
 
    }catch(error){      //走到这里说明Promise那边的回调是异常情况，说明发送邮箱失败
        console.log("catch error is ", error)

        callback(null, { email:  call.request.email,
            error:const_module.Errors.Exception
        }); 
    }
     
}

function main() {
    var server = new grpc.Server()
    server.addService(message_proto.VarifyService.service, { GetVarifyCode: GetVarifyCode })//前一个GetVarifyCode是string，我们要的服务名。后面的是接口，上面写的GetVarifyCode函数
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        server.start()
        console.log('grpc server started')        
    })
}

main()