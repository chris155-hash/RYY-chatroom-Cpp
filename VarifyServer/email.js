const nodemailer = require('nodemailer');
const config_module = require("./config")

/**
 * 创建发送邮件的代理
 */
let transport = nodemailer.createTransport({
    host: 'smtp.163.com',
    port: 465,
    secure: true,
    auth: {
        user: config_module.email_user, // 发送方邮箱地址
        pass: config_module.email_pass // 邮箱授权码或者密码
    }
});

/**
 * 发送邮件的函数
 * @param {*} mailOptions_ 发送邮件的参数
 * @returns 
 */
function SendMail(mailOptions_){
    return new Promise(function(resolve, reject){    //这里Sendmail无论发没发成功都会返回，异步编程，那我们这里向变成同步，确保发送成功，用了Promise，类似C++的futuer。
        transport.sendMail(mailOptions_, function(error, info){
            if (error) {    //异常情况的回调
                console.log(error);
                reject(error);
            } else {        //正常情况的回调 
                console.log('邮件已成功发送：' + info.response);
                resolve(info.response)
            }
        });
    })
   
}

module.exports.SendMail = SendMail