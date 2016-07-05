var emg = []; // dataPoints Acc Sensor1
var dpsAccYS1 = []; // dataPoints Acc Sensor1
var dpsAccZS1 = []; // dataPoints Acc Sensor1
var dpsGyroXS1 = []; // dataPoints Gyro Sensor1
var dpsGyroYS1 = []; // dataPoints Gyro Sensor1
var dpsGyroZS1 = []; // dataPoints Gyro Sensor1
var dpsAccXS2 = []; // dataPoints Acc Sensor1
var dpsAccYS2 = []; // dataPoints Acc Sensor1
var dpsAccZS2 = []; // dataPoints Acc Sensor1
var dpsGyroXS2 = []; // dataPoints Gyro Sensor1
var dpsGyroYS2 = []; // dataPoints Gyro Sensor1
var dpsGyroZS2 = []; // dataPoints Gyro Sensor1
var dataLength = 300; // number of dataPoints visible at any point
var chart;
var s1GyroChart;
var s2AccChart;
var s2GyroChart;
var ws;
var banPreview = false;
var banSave = false;

window.onload = function () {
	chart = new CanvasJS.Chart("emgPlot",{
		backgroundColor: "transparent",
		axisY:{
			gridColor: "rgba(255,255,255,.05)",
			tickColor: "rgba(255,255,255,.05)",
			labelFontColor: "#a2a2a2"
		},
		axisX:{
			labelFontColor: "#a2a2a2"
		},
		data:[{
			type: "line",
			showInLegend: true,
			name: "X",
			dataPoints: emg
		},{
			type: "line",
			showInLegend: true,
			name: "Y",
			dataPoints: dpsAccYS1
		},{
			type: "line",
			showInLegend: true,
			name: "Z",
			dataPoints: dpsAccZS1
		}],
		legend: {
			cursor:"pointer",
			itemclick : function(e) {
				if (typeof(e.dataSeries.visible) === "undefined" || e.dataSeries.visible) {
				e.dataSeries.visible = false;
				}
				else {
					e.dataSeries.visible = true;
				}
				chart.render();
			}
		}
	});
	setupWs();
}

function setupWs(){
	var host = window.document.location.host.replace(/:.*/, '');
  ws = new WebSocket('ws://' + host + ':8080');

  ws.onmessage = function (event) {
		if(event.data == "fileRdy"){
			NProgress.done();
			$('#btnDownload').attr('disabled', false);
			$('#btnDownload').unbind('click', false);
			swal("File ready!", "You download the data using the button below the charts", "success")
		}else{
			var lecture = JSON.parse(event.data);
			emg.push({
				x: lecture.cont,
				y: lecture.value
			});
			if (emg.length == dataLength){
				emg.shift();
			}
			chart.render();
		}
  };
}

function startPreview(){
	banPreview = !banPreview;
	if (banPreview){
		$("#btnStartPreview").html('Stop data');
	}else{
		$("#btnStartPreview").html('Start data');
	}
	ws.send("startPreview");
}
