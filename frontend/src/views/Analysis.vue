<template>
    <v-container>
        <!-- row 1 -->
        <v-row>
            <!-- col 1 -->
            <v-col>
                <v-text-field v-model="start" label="Start date" type="Date" dense solo-inverted class="mr-5" :style="{ maxWidth: '300px' }" flat></v-text-field>
                <v-text-field v-model="end" label="End date" type="Date" dense solo-inverted :style="{ marginRight: '5px', maxWidth: '300px' }" flat></v-text-field>
                <v-btn text="Analyze" @click=" updateCards(); updateLineChart();"></v-btn>
                <br />
                <br />
                <br />
            </v-col>

            <!-- col 2 -->
            <v-col>
                <v-sheet width="200px">
                    <v-card class="mb-5" style="max-width: 200px" color="surface" title="Average" subtitle="For the selected period">
                        <span class="text-onSecondaryContainer text-h3">{{ reserve }}</span>Gal
                    </v-card>
                </v-sheet>
            </v-col>

            <!-- col 3 -->
            <v-col></v-col>
        </v-row>
        
        <!-- row 2 -->
        <v-row>
            <v-col>
                <figure class="highcharts-figure">
                    <div id="container0"></div>
                </figure>
            </v-col>
        </v-row>

        <!-- row 3 -->
        <v-row>
            <v-col>
                <figure class="highcharts-figure">
                    <div id="container1"></div>
                </figure>
            </v-col>
        </v-row>
    </v-container>
</template>
  
<script setup>
/** JAVASCRIPT HERE */
import Highcharts from "highcharts";
import more from "highcharts/highcharts-more";
import Exporting from "highcharts/modules/exporting";
import { withDirectives } from "vue";
Exporting(Highcharts);
more(Highcharts);
  
// IMPORTS
 
import { useMqttStore } from "@/store/mqttStore"; // Import Mqtt Store
import { storeToRefs } from "pinia";
  
import { useAppStore } from "@/store/appStore";
import { ref, reactive, watch, onMounted, onBeforeUnmount, computed, } from "vue";
import { useRoute, useRouter } from "vue-router";
  
// VARIABLES
const router        = useRouter();
const route         = useRoute();
const Mqtt          = useMqttStore();
const AppStore      = useAppStore();
const lineChart     = ref(null);        // Line Chart object
const scatterChart  = ref(null);        // Scatter Chart object
var start           = ref(null);
var end             = ref(null);
var reserve         = ref(null);

onMounted(() => {
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    CreateCharts();

    Mqtt.connect(); // Connect to Broker located on the backend
    setTimeout(() => {
        // Subscribe to each topic
        Mqtt.subscribe("620151149");
        Mqtt.subscribe("620151149_sub");
    }, 3000);
});

onBeforeUnmount(() => {
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
    Mqtt.unsubcribeAll();
});

const CreateCharts = async () => {
    // LINE CHART FOR RESERVE VARIABLE
    lineChart.value = Highcharts.chart("container0", {
        chart: { zoomType: "x" },
        title: { text: "Water Management Analysis", align: "left" },
        yAxis: {
            title: { text: "Water Reserve", style: { color: "#000000" } },
            labels: { format: "{value} Gal" },
        },
    
        tooltip: {
            pointFormat: "Time: {point.x} s <br/> Water Reserve: {point.y} Gal",
        },
        
        xAxis: {
            type: "datetime",
            title: { text: "Time", style: { color: "#000000" } },
        },
        
        tooltip: { shared: true },
        series: [
            {
                name: "Water Reserve",
                type: "line",
                data: [],
                turboThreshold: 0,
                color: Highcharts.getOptions().colors[0],
            },
        ],
    });
  
    scatterChart.value = Highcharts.chart("container1", {
        chart: { zoomType: "x" },
        title: { text: "Height Level and Water Level Correlation Analysis", align: "left" },
        yAxis: {
            title: {
                text: "Height",
                style: { color: "#000000" },
            },
            labels: { format: "{value} in" },
        },
  
        xAxis: {
            title: { text: "Water Height", style: { color: "#000000" } },
            labels: { format: "{value} in" },
        },
        tooltip: {
            shared: true,
            pointFormat: "Water Height: {point.x} in <br/> Height: {point.y} in",
        },
        series: [
            {
                name: "Analysis",
                type: "scatter",
                data: [],
                turboThreshold: 0,
                color: Highcharts.getOptions().colors[0],
            },
        ],
        });
};
  
// FUNCTIONS
const updateCards = async () => {
    if (!!start.value && !!end.value) {
        // 1. Convert start and end dates collected fron TextFields to 10 digit timestamps
        let startDate = new Date(start.value).getTime() / 1000;
        let endDate = new Date(end.value).getTime() / 1000;
        // 2. Fetch data from backend by calling the API functions
        const Gallon = await AppStore.update_Card(startDate, endDate);
        console.log(Gallon[0].average.toFixed(1));
        reserve.value = Gallon[0].average.toFixed(1);
    }
};
  
const updateLineChart = async () => {
    if (!!start.value && !!end.value) {
        // Convert output from Textfield components to 10 digit timestamps
        let startDate = new Date(start.value).getTime() / 1000;
        let endDate = new Date(end.value).getTime() / 1000;
        // Fetch data from backend
        const data = await AppStore.getReserve(startDate, endDate);
        // Create arrays for each plot
        let reserve = [];
        let scatter = [];
        // Iterate through data variable and transform object to format recognized by highcharts
        data.forEach((row) => {
        reserve.push({
            x: row.timestamp * 1000,
            y: parseFloat(row.reserve.toFixed(2)),
        });
        });
        data.forEach((row) => {
        scatter.push({
            x: parseFloat(row.waterheight.toFixed(2)),
            y: parseFloat(row.radar.toFixed(2)),
        });
        });
        // Add data to line and scatter chart
        lineChart.value.series[0].setData(reserve);
        scatterChart.value.series[0].setData(scatter);
    }
};
</script>

<style scoped>
/** CSS STYLE HERE */
</style>