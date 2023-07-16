// Create the charts when the web page loads
window.addEventListener('load', onload);

function onload(event){
  chartHR = createhrChart();
  chartRR = createrrChart();
  chartSPO = createspoChart();
  chartSYS = createsysChart();
  chartDIA = creatediaChart();
}

// Create Heart Rate Chart
function createhrChart() {
  var chart = new Highcharts.Chart({
    chart:{ 
      renderTo:'chart-hr',
      type: 'spline' 
    },
    series: [
      {
        name: 'Pengukuran'
      }
    ],
    title: { 
      text: undefined
    },
    plotOptions: {
      line: { 
        animation: false,
        dataLabels: { 
          enabled: true 
        }
      }
    },
    xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
      title: { 
        text: 'Heart Rate (BPM)' 
      }
    },
    credits: { 
      enabled: false 
    }
  });
  return chart;
}

// Create Respiration Rate Chart
function createrrChart(){
  var chart = new Highcharts.Chart({
    chart:{ 
      renderTo:'chart-rr',
      type: 'spline'  
    },
    series: [{
      name: 'Pengukuran'
    }],
    title: { 
      text: undefined
    },    
    plotOptions: {
      line: { 
        animation: false,
        dataLabels: { 
          enabled: true 
        }
      },
      series: { 
        color: '#50b8b4' 
      }
    },
    xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
      title: { 
        text: 'Respiration Rate' 
      }
    },
    credits: { 
      enabled: false 
    }
  });
  return chart;
}

// Create SpO2 Chart
function createspoChart() {
  var chart = new Highcharts.Chart({
    chart:{ 
      renderTo:'chart-spo',
      type: 'spline'  
    },
    series: [{
      name: 'Pengukuran'
    }],
    title: { 
      text: undefined
    },    
    plotOptions: {
      line: { 
        animation: false,
        dataLabels: { 
          enabled: true 
        }
      },
      series: { 
        color: '#A62639' 
      }
    },
    xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
      title: { 
        text: 'SpO2 (%)' 
      }
    },
    credits: { 
      enabled: false 
    }
  });
  return chart;
}

// Create Systolik Chart
function createsysChart() {
  var chart = new Highcharts.Chart({
    chart:{ 
      renderTo:'chart-sys',
      type: 'spline'  
    },
    series: [{
      name: 'Pengukuran'
    }],
    title: { 
      text: undefined
    },    
    plotOptions: {
      line: { 
        animation: false,
        dataLabels: { 
          enabled: true 
        }
      },
      series: { 
        color: '#A62639' 
      }
    },
    xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
      title: { 
        text: 'Systolik (BPM)' 
      }
    },
    credits: { 
      enabled: false 
    }
  });
  return chart;
}

// Create Diastolik Chart
function creatediaChart() {
  var chart = new Highcharts.Chart({
    chart:{ 
      renderTo:'chart-dia',
      type: 'spline'  
    },
    series: [{
      name: 'Pengukuran'
    }],
    title: { 
      text: undefined
    },    
    plotOptions: {
      line: { 
        animation: false,
        dataLabels: { 
          enabled: true 
        }
      },
      series: { 
        color: '#A62639' 
      }
    },
    xAxis: {
      type: 'datetime',
      dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
      title: { 
        text: 'Diastolik (BPM)' 
      }
    },
    credits: { 
      enabled: false 
    }
  });
  return chart;
}