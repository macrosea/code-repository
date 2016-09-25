
var http = require('http');
var util = require('util');
var querystring = require('querystring');
var parseString = require('xml2js').parseString;
var print = console.log



var prices = {};
var unitValues = {};


var watchAndCalc = function (){
    return;
    if (prices.length ==2 && unitValues.length == 3)
    {
        var pc = prices[0] + prices[1];

    }

};



var getPrice = function(code, cntxt){
    if (cntxt == null)
        return;

    var res = querystring.parse(cntxt.replace(/ /g, ''), ';');
    var price = res['dataArr'].slice(2, -2).split(',')[2];
    prices[code] = price; 
    console.log(JSON.stringify(prices), 'utf8');
    watchAndCalc();
}

// (new Date()).Format("yyyy-MM-dd hh:mm:ss.S") ==> 2006-07-02 08:09:04.423 
// (new Date()).Format("yyyy-M-d h:m:s.S")      ==> 2006-7-2 8:9:4.18 
Date.prototype.toFormat = function (fmt) { //author: meizz 
    var o = {
        "M+": this.getMonth() + 1, //月份 
        "d+": this.getDate(), //日 
        "h+": this.getHours(), //小时 
        "m+": this.getMinutes(), //分 
        "s+": this.getSeconds(), //秒 
        "q+": Math.floor((this.getMonth() + 3) / 3), //季度 
        "S": this.getMilliseconds() //毫秒 
    };
    if (/(y+)/.test(fmt)) fmt = fmt.replace(RegExp.$1, (this.getFullYear() + "").substr(4 - RegExp.$1.length));
    for (var k in o)
    if (new RegExp("(" + k + ")").test(fmt)) fmt = fmt.replace(RegExp.$1, (RegExp.$1.length == 1) ? (o[k]) : (("00" + o[k]).substr(("" + o[k]).length)));
    return fmt;
}

var getUnitValue = function(code, cntxt){
    //print(cntxt);
    parseString(cntxt, function (err, result) {
        var yesterday = (new Date((new Date()).getTime() - 1*24*60*60*1000)).toFormat("yyyy-MM-dd");
        print(JSON.stringify(result));
        return;
        datum = result.DataSet.Data;
        datum.forEach( function(em) {
            //print(JSON.stringify(em));
            if (em.fld_enddate == yesterday) {
                unitValues[code] = em['fld_unitnetvalue'][0]; 
                print(JSON.stringify(unitValues), 'utf8');
                watchAndCalc();
                return;
            }
        });
    });
}

var fetchData = function(fund, cb){
    http.get(fund.url, (res) => {
        console.log('statusCode: ', res.statusCode);
        //console.log('headers: ', res.headers);
        //res.setEndcode('utf8');
        res.on('data', (body) => {
            //process.stdout.write(body);
            cntxt = body.toString('utf8');
            //console.log(cntxt, 'utf8');
            return cb(fund.code, cntxt);
        });
        
    }).on('error', (e) => {
        console.error(e);
        return cb(fund.code, null);
    });
}

var fundGroup = {'A':150018, 'B':150019, 'C': 161812};

var codes1 = [fundGroup.A, fundGroup.B];
codes1.forEach(function(code){
    var url = 'http://quote.stock.hexun.com/stockdata/fund_quote.aspx?stocklist='+code;
    fetchData({'code':code, 'url':url}, getPrice);
});

var codes2 =  [fundGroup.A, fundGroup.B, fundGroup.C];
codes2.forEach(function(code) {
    var dtNow=new Date();
    var end = (new Date(dtNow.getTime() + 1*24*60*60*1000)).toFormat("yyyy-MM-dd");
    var start = (new Date(dtNow.getTime() - 2*24*60*60*1000)).toFormat("yyyy-MM-dd");
    var url = `http://data.funds.hexun.com/outxml/detail/openfundnetvalue.aspx?fundcode=${code}&startdate=${start}&enddate=${end}`;
    fetchData({'code':code, 'url':url}, getUnitValue);

});




/*

*/

console.log("hello...........");
