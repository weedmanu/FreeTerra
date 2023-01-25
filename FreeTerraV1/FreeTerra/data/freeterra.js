var TargetTemp;
var tempC;
var humiC;
var tempF;
var humiF;
var date;
var heure;
var TargetTempJ;
var TargetTempN;
var HysTemp;
var TempMax;
var TempMin;
var Lever_H;
var Lever_M;
var Coucher_H;
var Coucher_M;

window.onload = function () {
	RecupConfig();
	RGraph.AJAX.getCSV({url: '/datas', callback: myCallback});
	setInterval(function() {
		RecupConfig();
		RGraph.AJAX.getCSV({url: '/datas', callback: myCallback});		
	}, 10000);
};

function myCallback (data) {

	RGraph.reset(document.getElementById('JaugeTempChaud'));
	RGraph.reset(document.getElementById('JaugeHumiChaud'));
	RGraph.reset(document.getElementById('JaugeTempFroid'));
	RGraph.reset(document.getElementById('JaugeHumiFroid'));		

	gauge = new RGraph.Gauge({
		id: 'JaugeTempChaud',
		min: 0,
		max: 50,
		value: data[3],
		options: {
			textColor: 'white',
			backgroundColor: 'black',
			borderOuter:     'grey',
			borderInner:     'black',
			borderOutline:   'black',
			shadow:           true,
			shadowColor:     'grey',
			centerpinColor:  'black',
			labelsValue: true,
			labelsValueDecimals: 1,
			labelsValueBounding: false,
			labelsValueYPos: 0.75,
			labelsValueBold: true,
			labelsValueUnitsPost: ' °C',
			titleTop: 'DHT22',
			titleBottom: 'Température',
			colorsRanges: [[0, 15, '#0000FF'], [15, 24,'#00FFFF'], [23, 30, '#00FF47'], [30, 35, '#FFFF00'], [35,50,'#FF4A00']] ,
			adjustable: true,
			textSize: 12
		}
	}).draw();

	gauge2 = new RGraph.Gauge({
		id: 'JaugeHumiChaud',
		min: 0,
		max: 100,
		value: data[4],
		options: {
			textColor: 'white',
			backgroundColor: 'black',
			borderOuter:     'grey',
			borderInner:     'black',
			borderOutline:   'black',
			shadow:           true,
			shadowColor:     'grey',
			centerpinColor:  'black',
			labelsValue: true,
			labelsValueDecimals: 1,
			labelsValueBounding: false,
			labelsValueYPos: 0.75,
			labelsValueBold: true,
			labelsValueUnitsPost: ' %',
			titleTop: 'DHT22',
			titleBottom: 'Humidité',
			colorsRanges: [[0, 40, '#DC3912'], [40, 50,'#FF9900'], [50, 75, '#00FF00'], [75, 85, '#FF9900'], [85,100,'#DC3912']] ,
			adjustable: true,
			textSize: 12
		}
	}).draw();

	gauge3 = new RGraph.Gauge({
		id: 'JaugeTempFroid',
		min: 0,
		max: 50,
		value:  data[5],
		options: {
			textColor: 'white',
			backgroundColor: 'black',
			borderOuter:     'grey',
			borderInner:     'black',
			borderOutline:   'black',
			shadow:           true,
			shadowColor:     'grey',
			centerpinColor:  'black',
			labelsValue: true,
			labelsValueDecimals: 1,
			labelsValueBounding: false,
			labelsValueYPos: 0.75,
			labelsValueBold: true,
			labelsValueUnitsPost: ' °C',
			titleTop: 'DHT22',
			titleBottom: 'Température',
			colorsRanges: [[0, 15, '#0000FF'], [15, 24,'#00FFFF'], [23, 30, '#00FF47'], [30, 35, '#FFFF00'], [35,50,'#FF4A00']] ,
			adjustable: true,
			textSize: 12
		}
	}).draw();

	gauge4 = new RGraph.Gauge({
		id: 'JaugeHumiFroid',
		min: 0,
		max: 100,
		value:  data[6],
		options: {
			textColor: 'white',
			backgroundColor: 'black',
			borderOuter:     'grey',
			borderInner:     'black',
			borderOutline:   'black',
			shadow:           true,
			shadowColor:     'grey',
			centerpinColor:  'black',
			labelsValue: true,
			labelsValueDecimals: 1,
			labelsValueBounding: false,
			labelsValueYPos: 0.75,
			labelsValueBold: true,
			labelsValueUnitsPost: ' %',
			titleTop: 'DHT22',
			titleBottom: 'Humidité',
			colorsRanges: [[0, 40, '#DC3912'], [40, 50,'#FF9900'], [50, 75, '#00FF00'], [75, 85, '#FF9900'], [85,100,'#DC3912']] ,
			adjustable: true,
			textSize: 12
		}
	}).draw();
	
	TargetTemp = data[2];
	tempC = data[3];
	humiC = data[4];
 	tempF = data[5];
	humiF = data[6];
	AlarmeSondePC = data[7];
	AlarmeSondePF = data[8];
	AlarmeTempMin = data[9];   
	AlarmeTempMax = data[10];
	Etat_Lum = data[11]; 
	Etat_Chauff = data[12]; 
	
	document.getElementById('TargetTemp').innerHTML = TargetTemp + ' °C';;
	document.getElementById('TargetTemp').style.color = '#FF4343';
	
	document.getElementById('tempC').innerHTML = tempC + ' °C';		
	if ( tempC < TargetTemp+1 && tempC > TargetTemp-1 ) { document.getElementById('tempC').style.color = '#C1FF6B';	} 
	else if ( tempC > TargetTemp+2 || tempC < TargetTemp-2 ) { document.getElementById('tempC').style.color = '#FFFF00'; }
	else if ( tempC > TempMax || tempC < TempMin ) { document.getElementById('tempC').style.color = '#FF7F7F'; }		
	else { document.getElementById('tempC').style.color = "#FF7F7F"; }
		
	document.getElementById('humiC').innerHTML = humiC + ' %';
	document.getElementById('humiC').style.color = "#FF7F7F"; 
	
	document.getElementById('tempF').innerHTML = tempF + ' °C';
	if ( tempF < TempMax && tempF > TempMin ) { document.getElementById('tempF').style.color = "#C1FF6B"; }
	else if ( tempF > TempMax || tempF < TempMin ) { document.getElementById('tempF').style.color = "#FF7F7F"; }
	else { document.getElementById('tempF').style.color = "#FF7F7F"; }	
	
	document.getElementById('humiF').innerHTML = humiF + ' %';
	document.getElementById('humiF').style.color = "#FF7F7F"; 
	
		
	if (AlarmeTempMax) { 
		img_AlarmeTemp = '<img src="\chaud.png" style="width:42px;height:42px;">';
		txt_AlarmeTemp = "<span class='clignote'>Il fait trop chaud !!!</span>"; 
		document.getElementById('img_AlarmeTemp').innerHTML = img_AlarmeTemp;
		document.getElementById('txt_AlarmeTemp').innerHTML = txt_AlarmeTemp;
		document.getElementById('txt_AlarmeTemp').style.color = '#FF7F7F';
	} else if (AlarmeTempMin) { 
		img_AlarmeTemp = '<img src="\froid.png" style="width:42px;height:42px;">';
		txt_AlarmeTemp = "<span class='clignote'>Il fait trop froid !!!</span>"; 
		document.getElementById('img_AlarmeTemp').innerHTML = img_AlarmeTemp;
		document.getElementById('txt_AlarmeTemp').innerHTML = txt_AlarmeTemp;
		document.getElementById('txt_AlarmeTemp').style.color = '#FF7F7F';
	} else { 
		img_AlarmeTemp = '<img src="\ok.png" style="width:42px;height:42px;">';
		txt_AlarmeTemp = "Température correct"; 
		document.getElementById('img_AlarmeTemp').innerHTML = img_AlarmeTemp;
		document.getElementById('txt_AlarmeTemp').innerHTML = txt_AlarmeTemp;
		document.getElementById('txt_AlarmeTemp').style.color = '#C1FF6B';
	}
	
	if (AlarmeSondePC) { 
		img_AlarmeSondePC = '<img src="\ko.png" style="width:42px;height:42px;">';
		txt_AlarmeSondePC = "<span class='clignote'>Erreur lecture sonde point chaud</span>"; 
		document.getElementById('img_AlarmeSondePC').innerHTML = img_AlarmeSondePC;
		document.getElementById('txt_AlarmeSondePC').innerHTML = txt_AlarmeSondePC;
		document.getElementById('txt_AlarmeSondePC').style.color = '#FF7F7F';
	} else { 
		img_AlarmeSondePC = '<img src="\ok.png" style="width:42px;height:42px;">';
		txt_AlarmeSondePC = "Lecture sonde point chaud OK"; 
		document.getElementById('img_AlarmeSondePC').innerHTML = img_AlarmeSondePC;
		document.getElementById('txt_AlarmeSondePC').innerHTML = txt_AlarmeSondePC;
		document.getElementById('txt_AlarmeSondePC').style.color = '#C1FF6B';
	}
	
	if (AlarmeSondePF) { 
		img_AlarmeSondePF = '<img src="\ko.png" style="width:42px;height:42px;">';
		txt_AlarmeSondePF = "<span class='clignote'>Erreur lecture sonde point froid</span>"; 
		document.getElementById('img_AlarmeSondePF').innerHTML = img_AlarmeSondePF;
		document.getElementById('txt_AlarmeSondePF').innerHTML = txt_AlarmeSondePF;
		document.getElementById('txt_AlarmeSondePF').style.color = '#FF7F7F';
	} else { 
		img_AlarmeSondePF = '<img src="\ok.png" style="width:42px;height:42px;">';
		txt_AlarmeSondePF = "Lecture sonde point froid OK"; 
		document.getElementById('img_AlarmeSondePF').innerHTML = img_AlarmeSondePF;
		document.getElementById('txt_AlarmeSondePF').innerHTML = txt_AlarmeSondePF;
		document.getElementById('txt_AlarmeSondePF').style.color = '#C1FF6B';
	}
}

function RecupConfig() {        
	var xhttp = new XMLHttpRequest();                            // on construit une requete
	xhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			// on traite la reponse, la découpe et met nos datas dans des tableaux           
			var datas = this.responseText;                           
			var split = datas.split(',');
			date = split[0];
			heure = split[1]; 
			TargetTempJ = split[2];
			TargetTempN = split[3];
			HysTemp = split[4];
			TempMax = split[5];
			TempMin= split[6];
			Lever_H = split[7]; 
			Lever_M = split[8]; 
			Coucher_H = split[9]; 
			Coucher_M = split[10]; 
			
			if   ( Lever_M < 10) {var Lever = Lever_H + "h0" + Lever_M; }
			else {var Lever =  Lever_H + "h" + Lever_M; }
			
			if   ( Coucher_M < 10) {var Coucher = Coucher_H + "H0" + Coucher_M; }
			else {var Coucher = Coucher_H + "H" + Coucher_M; }
			
			var message = "Datas reçues le " + date + " à " + heure;
			document.getElementById('instant').innerHTML = message;		
			
			document.getElementById('TargetTempJ').innerHTML = TargetTempJ + ' °C';
			document.getElementById('TargetTempJ').style.color = '#FF4343';
			
			document.getElementById('TargetTempN').innerHTML = TargetTempN + ' °C';
			document.getElementById('TargetTempN').style.color = '#FF4343';
			
			document.getElementById('HysTemp').innerHTML = HysTemp + ' °C';
			document.getElementById('HysTemp').style.color = '#FF4343';
			
			document.getElementById('TempMax').innerHTML = TempMax + ' °C';
			document.getElementById('TempMax').style.color = '#FF4343';
			
			document.getElementById('TempMin').innerHTML = TempMin + ' °C';		
			document.getElementById('TempMin').style.color = '#FF4343';							
			
			document.getElementById('Lever').innerHTML = Lever;	
			document.getElementById('Lever').style.color = '#FFFB3F';
			
			document.getElementById('Coucher').innerHTML = Coucher;	
			document.getElementById('Coucher').style.color = '#FFFB3F';	
		}
	};
	xhttp.open("GET", "/data_config", true); 		// requete pour ouvrir la page readDHT du serveur pour récupérer nos datas
	xhttp.send();                       			// on envoie la requete                     
}


