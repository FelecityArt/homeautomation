<template>
    <v-container>
        <!-- row 1 -->
        <v-row>
            <!-- col 1 -->
            <v-col cols="1">
                <v-sheet>
                    <v-slider direction="vertical" v-model="radar" :max="94.5" :min="0" step="0.1" thumb-label="always" color="green" label="Height (in)" track-size="50"></v-slider>
                </v-sheet>
            </v-col>
            
            <!-- col 2 -->
            <v-col justify="left" cols="11">
                <v-sheet>
                    <figure class="highcharts-figure">
                        <div id="container0"></div>
                    </figure>
                </v-sheet>
            </v-col>
        </v-row>

        <!-- row 2 -->
        <v-row>
            <!-- col 1 -->
            <v-col cols="8">
                <v-sheet>
                    <figure class="highcharts-figure">
                        <div id="container1"></div>
                    </figure>
                </v-sheet>
            </v-col>

            <!-- col 2 -->
            <v-col cols="4">
                <v-sheet max-width="350px">
                    <v-card class="mb-5" style="max-width: 350px" variant="outlined" color="primary" density="compact" rounded="lg" title="Water Level" subtitle="Capacity Remaining">
                        <div id="container2"></div>
                    </v-card>
                </v-sheet>
            </v-col>
        </v-row>

        <v-dialog width="500" v-model="isActive">
            <template v-slot:default="{ isActive }">
                <v-card title="WARNING !" color="warning" background-color="primary darken-1">
                    <v-card-text>Tank is over maximum capacity. Open overflow valve now to resolve issue.</v-card-text> 
                    <v-card-actions>
                        <v-spacer></v-spacer>
                    </v-card-actions>
                </v-card>
            </template>
        </v-dialog>
    </v-container>
</template>
  
<script setup>
/** JAVASCRIPT HERE */
  
// IMPORTS
import { useMqttStore } from "@/store/mqttStore"; // Import Mqtt Store
import { storeToRefs } from "pinia";

const Mqtt = useMqttStore();
const { payload, payloadTopic } = storeToRefs(Mqtt);
  
// Highcharts, Load the exporting module and Initialize exporting module.
import Highcharts from "highcharts";
import more from "highcharts/highcharts-more";
import Exporting from "highcharts/modules/exporting";
Exporting(Highcharts);
more(Highcharts);
  
import { ref, reactive, watch, onMounted, onBeforeUnmount, computed, } from "vue";
import { useRoute, useRouter } from "vue-router";
  
// VARIABLES
const router        = useRouter();
const route         = useRoute();
const areaGraph     = ref(null);    // areaGraph object
const gauge         = ref(null);    // gauge object
const percentage    = ref(null);
const points        = ref(600);
const shift         = ref(false);
var isActive        = ref(null);
var radar           = ref(null);
var fm              = new FluidMeter();

onMounted(() => {
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    CreateCharts();

    Mqtt.connect();
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


// WATCHERS
watch(payload, (data) => {
    fm.setPercentage(data.percentage);
    radar.value = data.radar;
    if (data.percentage > 100) {
      isActive.value = true;
    } else {
      isActive.value = false;
    }
    console.log(data.percentage);
    if (points.value > 0) {
      points.value--;
    } else {
      shift.value = true;
    }
    
    areaGraph.value.series[0].addPoint({y:parseFloat(data.reserve.toFixed(2)), x: data.timestamp * 1000 }, true, shift.value);
    gauge.value.series[0].points[0].update(parseFloat(data.reserve));
});

// FUNCTIONS
const CreateCharts = async () => {
    // AREA GRAPH FOR RESERVE VARIABLE:
    areaGraph.value = Highcharts.chart("container0", {
        chart: { zoomType: "x" },
        title: { text: "Water Reserves (10 min)", align: "left" },
        yAxis: {
            title: { text: "Water Level", style: { color: "#000000" } },
            labels: { format: "{value} Gal" },
        },
        xAxis: {
            type: "datetime",
            title: { text: "Time", style: { color: "#000000" } },
        },
        tooltip: { shared: true },
        series: [
            {
                name: "Water",
                type: "area",
                data: [],
                turboThreshold: 0,
                color: Highcharts.getOptions().colors[0],
            },
        ],
    });

    // GAUGE GRAPH FOR RESERVE VARIABLE:
    gauge.value = Highcharts.chart("container1", {
        title: { text: "Water Reserves", align: "left" },
        yAxis: { 
            min: 0,
            max: 1000,
            tickPixelInterval: 72,
            tickPosition: "inside",
            tickColor: Highcharts.defaultOptions.chart.backgroundColor || "#FFFFFF",
            tickLength: 20,
            tickWidth: 2,
            minorTickInterval: null,
            labels: { distance: 20, style: { fontSize: "14px" } },
            lineWidth: 0,
            plotBands: [
                { from: 0, to: 200, color: "#DF5353", thickness: 20 },
                { from: 200, to: 600, color: "#DDDF0D", thickness: 20 },
                { from: 600, to: 1000, color: "#55BF3B", thickness: 20 },
            ],
        },
        tooltip: { shared: true },
        pane: {
            startAngle: -90,
            endAngle: 89.9,
            background: null,
            center: ["50%", "75%"],
            size: "110%",
        },
        series: [
            {
                type: "gauge",
                name: "Water Capacity",
                data: [0],
                tooltip: { valueSuffix: " Gal" },
                dataLabels: {
                    format: "{y} Gal",
                    borderWidth: 0,
                    color: (Highcharts.defaultOptions.title && Highcharts.defaultOptions.title.style && Highcharts.defaultOptions.title.style.color) || "#333333",
                    style: { fontSize: "16px" },
                },
                dial: {
                    radius: "80%",
                    backgroundColor: "gray",
                    baseWidth: 12,
                    baseLength: "0%",
                    rearLength: "0%",
                },
                pivot: { backgroundColor: "gray", radius: 6 },
            },
        ],
    });

    fm.init({
        targetContainer: document.getElementById("container2"),
        fillPercentage: percentage,
        options: {
            fontSize: "70px",
            fontFamily: "Arial",
            fontFillStyle: "white",
            drawShadow: true,
            drawText: true,
            drawPercentageSign: true,
            drawBubbles: true,
            size: 300,
            borderWidth: 25,
            backgroundColor: "#e2e2e2",
            foregroundColor: "#fafafa",
            foregroundFluidLayer: {
                fillStyle: "purple",
                angularSpeed: 100,
                maxAmplitude: 12,
                frequency: 30,
                horizontalSpeed: -150,
            },
            backgroundFluidLayer: {
                fillStyle: "pink",
                angularSpeed: 100,
                maxAmplitude: 9,
                frequency: 30,
                horizontalSpeed: 150,
            },
        },
    });
};
</script>
  
<style scoped>
/** CSS STYLE HERE */
figure {
    border: 2px solid black;
}
</style>