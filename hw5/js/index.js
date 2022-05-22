var chart;
var inputPoints = [];
var subdivideMode = "Chaikin"; // Chaikin, AvgThreePowerBSpline, 4Points
var depth = 3;
var alpha = 0.1;

var defaultOption = {
    xAxis: {
        type: 'value',
        min: 0,
        max: 1,
        axisLine: {
            show: true,
            symbol: ['none', 'arrow']
        }
    },
    yAxis: {
        type: 'value',
        min: 0,
        max: 1,
        axisLine: {
            show: true,
            symbol: ['none', 'arrow']
        },
        axisLabel: {
            formatter: function (value, index) {
                return value.toFixed(3);
            }
        }
    },
}

window.onload = function(){
    var chartElement = document.getElementById("chart");
    chart = echarts.init(chartElement);
    chart.setOption(defaultOption);

    // 事件处理
    chartElement.onclick = function(event){
        if (draggingIndex != -1){
            draggingIndex = -1;
            return;
        }
        var screenPoint = [event.offsetX, event.offsetY]
        if (!chart.containPixel({gridIndex: 0}, screenPoint)) return;
        var point = chart.convertFromPixel({gridIndex: 0}, screenPoint);
        inputPoints.push(point);
        draw();
    }

    // 拖拽顶点
    var draggingIndex = -1;
    chart.on('mousedown', function (params) {
        draggingIndex = params.dataIndex;
    });

    window.onmousemove = function (event) {
        if (draggingIndex != -1){
            var screenPoint = [event.offsetX, event.offsetY]
            if (!chart.containPixel({gridIndex: 0}, screenPoint)) return;
            inputPoints[draggingIndex] = chart.convertFromPixel({gridIndex: 0}, screenPoint);
            draw();
        }
    };
}

window.onresize = function(){
    chart.resize();
}

window.onkeydown = function (event) {
    console.log(event)
    if (event.key == 'r'){
        reset();
    }
};

function reset(){
    inputPoints = [];
    chart.clear();
    chart.setOption(defaultOption);
}

function draw(){
    var points = inputPoints.slice(0);
    var series = [];
    // 散点
    var scatter = {
        name: 'pin',
        type: 'scatter',
        data: points,
        itemStyle:{  
            color: 'red'
        },
        tooltip: {
            show: true,
            formatter: function (params, ticket, callback) {
                return '( ' + params.data[0].toFixed(5) + ', ' + params.data[1].toFixed(5) + ' )';
            }
        },
        animation: false
    };
    
    series.push(scatter);
    // 凸包
    var convexLines = inputPoints.slice(0);
    convexLines.push(convexLines[0]);
    var convex = {
        type: 'line',
        data: convexLines,
        symbol: 'none',
        hoverAnimation: false,
        animation: false,
        lineStyle:{
            color: "#aaa",
            width: 1,
            type: "dashed"
        }
    };
    series.push(convex);
    // 曲线
    var lines;
    switch(subdivideMode){
        case "Chaikin": {
            lines = subdivideChaikin(points, depth);
            break;
        }
        case "AvgThreePowerBSpline": {
            lines = subdivideAvg3B(points, depth);
            break;
        }
        case "4Points": {
            lines = subdivide4Points(points, depth);
            break;
        }
    }
    var line = {
        type: 'line',
        data: lines,
        symbol: 'none',
        hoverAnimation: false,
        animation: false
    };
    series.push(line);

    var option = {series: series};
    chart.setOption(option);
}

function mix(p1, p2, u, v){
    if (!v) v = 1 - u;
    return [p1[0] * u + p2[0] * v, p1[1] * u + p2[1] * v];
}

function mix3(p1, p2, p3, u, v, w){
    return [p1[0] * u + p2[0] * v + p3[0] * w, p1[1] * u + p2[1] * v + p3[1] * w];
}

function subdivideChaikin(points, depth){
    if (depth == 0) {
        points.push(points[0]); // loop
        return points;
    }
    var res = [];
    for(var i = 0; i < points.length; i++){
        var p1 = points[i];
        var p2 = points[(i+1)%points.length];
        res.push(mix(p1, p2, 0.75));
        res.push(mix(p1, p2, 0.25));
    }
    return subdivideChaikin(res, depth - 1);
}

function subdivideAvg3B(points, depth){
    if (depth == 0) {
        points.push(points[0]); // loop
        return points;
    }
    var res = [];
    for(var i = 0; i < points.length; i++){
        var p0 = points[(i-1+points.length)%points.length];
        var p1 = points[i];
        var p2 = points[(i+1)%points.length];
        res.push(mix3(p0, p1, p2, 0.125, 0.75, 0.125));
        res.push(mix(p1, p2, 0.5));
    }
    return subdivideAvg3B(res, depth - 1);
}

function subdivide4Points(points, depth){
    if (depth == 0 || points.length < 4) {
        points.push(points[0]); // loop
        return points;
    }
    var newPoints = [];
    for(var i = 0; i < points.length; i++){
        var p0 = points[(i-1+points.length)%points.length];
        var p1 = points[i];
        var p2 = points[(i+1)%points.length];
        var p3 = points[(i+2)%points.length];

        var center1 = mix(p0, p3, 0.5);
        var center2 = mix(p1, p2, 0.5);
        var direction = [center2[0] - center1[0], center2[1] - center1[1]];
        var extra = [direction[0] * alpha, direction[1] * alpha];
        var newPoint = [center2[0] + extra[0], center2[1] + extra[1]];
        newPoints.push(newPoint);
    }
    var res = points.slice(0);
    for(var i = 0; i < points.length; i++){
        res.splice(2*i+1, 0, newPoints[i]);
    }

    return subdivide4Points(res, depth - 1);
}

function depthChange(newDepth){
    depth = newDepth;
    document.getElementById("depthText").innerText = "递归次数：" + depth + "次";
    draw();
}

function modeChange(newMode){
    subdivideMode = newMode;
    draw();
}