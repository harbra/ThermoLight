<!--
This vue.js file is the main display for detected IR commands.
-->
<template>
<div id="app" class="is-marginless">
  <TopNav title="Overview" subtitle="Device overview and display settings." menutitle="Overview" menutitleicon="thermometer"></TopNav>
  <section class="section">
    <nav class="level">
      <div class="level-item has-text-centered">
        <div>
          <p class="heading">Temperature</p>
          <p class="title">{{sensorvalues.temperature}}°C</p>
        </div>
      </div>
      <div class="level-item has-text-centered">
        <div>
          <p class="heading">Humidty</p>
          <p class="title">{{sensorvalues.humidity}}%</p>
        </div>
      </div>
      <div class="level-item has-text-centered">
        <div>
          <p class="heading">eCO2</p>
          <p class="title">{{sensorvalues.eco2}}ppm</p>
        </div>
      </div>
      <div class="level-item has-text-centered">
        <div>
          <p class="heading">TVOC</p>
          <p class="title">{{sensorvalues.tvoc}}ppb</p>
        </div>
      </div>
    </nav>

    <form v-on:submit.prevent="saveSettings()">
      <p class="title is-3">Brightness</p>
      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">Source</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <span class="select">
                <select v-model="brightness[0]" @change="brightnesssourcechanged()">
                  <option v-for="source in sourcesbrightness">{{source}}</option>
                </select>
              </span>
            </p>
          </div>
        </div>
      </div>
      <template v-if="brightness.length>0 && brightness[0]==='fixed'">
        <div class="field is-horizontal">
          <div class="field-label is-normal">
            <label class="label">Value</label>
          </div>
          <div class="field-body">
            <div class="field has-addons">
              <p class="control is-expanded">
                <input class="slider is-fullwidth is-white-gradient" step="1" min="0" max="255" v-model.number="brightness[1]" type="range" />
              </p>
              <p class="control">
                <input class="input" type="number" step="1" min="0" max="255" v-model.number="brightness[1]" style="width: 70px" />
              </p>
            </div>
          </div>
        </div>
      </template>
      <template v-else-if="brightness.length>0 && brightness[0]==='MQTT'">
        <div class="field is-horizontal">
          <div class="field-label is-normal">
            <label class="label">Topic</label>
          </div>
          <div class="field-body">
            <div class="field">
              <p class="control">
                <input class="input" type="text" placeholder="/topicparta/topicpartb" v-model="brightness[1]">
              </p>
            </div>
          </div>
        </div>

        <div class="field is-horizontal">
          <div class="field-label is-normal">
            <label class="label">Minimum brightness</label>
          </div>
          <div class="field-body">
            <div class="field has-addons">
              <p class="control is-expanded">
                <input class="slider is-fullwidth is-white-gradient" step="1" min="0" max="255" v-model.number="brightness[2]" type="range" />
              </p>
              <p class="control">
                <input class="input" type="number" min="0" max="255" v-model.number="brightness[2]" style="width: 70px" />
              </p>
            </div>
          </div>
        </div>
        <div class="field is-horizontal">
          <div class="field-label is-normal">
            <label class="label">Maximum brightness</label>
          </div>
          <div class="field-body">
            <div class="field has-addons">
              <p class="control is-expanded">
                <input class="slider is-fullwidth is-white-gradient" step="1" min="0" max="255" v-model.number="brightness[3]" type="range" />
              </p>
              <p class="control">
                <input class="input" type="number" min="0" max="255" v-model.number="brightness[3]" style="width: 70px" />
              </p>
            </div>
          </div>
        </div>

        <div class="field is-horizontal">
          <div class="field-label is-normal">
            <label class="label">MQTT input for minimum brightness</label>
          </div>
          <div class="field-body">
            <div class="field has-addons">
              <p class="control is-expanded">
                <input class="slider is-fullwidth is-white-gradient" step="any" v-model.number="brightness[4]" type="range" />
              </p>
              <p class="control">
                <input class="input" type="number" step="any" v-model.number="brightness[4]" style="width: 120px" />
              </p>
            </div>
          </div>
        </div>
        <div class="field is-horizontal">
          <div class="field-label is-normal">
            <label class="label">MQTT input for maximum brightness</label>
          </div>
          <div class="field-body">
            <div class="field has-addons">
              <p class="control is-expanded">
                <input class="slider is-fullwidth is-white-gradient" step="any" v-model.number="brightness[5]" type="range" />
              </p>
              <p class="control">
                <input class="input" type="number" step="any" v-model.number="brightness[5]" style="width: 120px" />
              </p>
            </div>
          </div>
        </div>

        <div class="field is-horizontal is-narrow">
          <div class="field-label">
            <label class="label">Night mode</label>
          </div>
          <div class="field-body">
            <div class="field">
              <p class="control">
                <input type="checkbox" v-model="brightness[6]"> Deactive if lower than minimum?
              </p>
            </div>
          </div>
        </div>
      </template>


      <p class="title is-3">Left side</p>
      <div class="columns is-mobile">
        <div class="column is-narrow">
          <svg height="100%" width="10px">
            <rect v-for="i in numelements"
               v-bind:style="ledcolors[numelements-1-(i-1)]"
               width="100%"
               v-bind:height="100/numelements+'%'"
               x="0"
               v-bind:y="(i-1)*(100/numelements)+'%'" />
          </svg>
        </div>

        <div class="column">
          <div class="columns">
            <div class="column is-half">
              <p class="title is-5">Primary</p>
              <div class="field is-horizontal">
                <div class="field-label">
                  <label class="label">Data source</label>
                </div>
                <div class="field-body">
                  <div class="field has-addons">
                    <div class="control is-expanded">
                      <div class="select is-fullwidth">
                        <select v-model="left.primary[0]">
                          <option v-for="source in sources">{{source}}</option>
                        </select>
                      </div>
                    </div>
                    <div class="control" v-if="left.primary[0]=='HTTP'">
                      <a class="button is-primary" v-on:click="showModalHTTP(left.primary,4)">
                        <span class="icon">
                          <i class="icon-gear"></i>
                        </span>
                        <span>Config</span>
                      </a>
                    </div>
                    <div class="control" v-else-if="left.primary[0]=='MQTT'">
                      <a class="button is-primary" v-on:click="showModalMQTT(left.primary,4)">
                        <span class="icon">
                          <i class="icon-gear"></i>
                        </span>
                        <span>Config</span>
                      </a>
                    </div>
                  </div>
                </div>
              </div>
              <template v-if="left.primary[0]!='none'">
                <div class="field is-horizontal" v-for="(value,idx) in left.primary[1]">
                  <div class="field-label">
                    <label class="label" v-if="idx==0">Lowest value</label>
                    <label class="label" v-else-if="idx==left.primary[1].length-1">Highest value</label>
                    <label class="label" v-else="idx==0"></label>
                  </div>
                  <div class="field-body">
                    <div class="field has-addons">
                      <div class="control" v-if="idx!=left.primary[1].length-1">
                        <a class="button is-success" v-on:click="left.primary[1].splice(idx+1,0,(left.primary[1][idx]+left.primary[1][idx+1])/2); left.primary[2].splice(idx+1,0,[255,255,255]); ">
                          <span class="icon">
                            <i class="icon-plus"></i>
                          </span>
                        </a>
                      </div>
                      <div class="control" v-if="idx!=0 && idx!=left.primary[1].length-1">
                        <a class="button is-danger" v-on:click="left.primary[1].splice(idx,1); left.primary[2].splice(idx,1); ">
                          <span class="icon">
                            <i class="icon-remove"></i>
                          </span>
                        </a>
                      </div>
                      <div class="control is-expanded">
                        <input class="input" type="number" step="any" placeholder="Upper bound" v-model.number="left.primary[1][idx]">
                      </div>
                      <div class="control" v-if="left.secondary[0]=='none'">
                        <a class="button" :style="getBackgroundStyle(left.primary[2][idx])" v-on:click="showModalColor(left.primary[2][idx])">
                          <span class="icon">
                            <i class="icon-eyedropper"></i>
                          </span>
                        </a>
                      </div>
                    </div>
                  </div>
                </div>
                <div class="field is-horizontal">
                  <div class="field-label">
                    <label class="label">Update interval</label>
                  </div>
                  <div class="field-body">
                    <div class="field has-addons">
                      <div class="control is-expanded">
                        <input class="input" type="number" step="1" placeholder="Update interval" v-model.number="left.primary[3]">
                      </div>
                      <div class="control">
                        <a class="button is-static">
                          ms
                        </a>
                      </div>
                    </div>
                  </div>
                </div>
              </template>
            </div>

            <div class="column">
              <p class="title is-5">Secondary</p>
              <div class="field is-horizontal">
                <div class="field-label">
                  <label class="label">Data source</label>
                </div>
                <div class="field-body">
                  <div class="field has-addons">
                    <div class="control is-expanded">
                      <div class="select is-fullwidth">
                        <select v-model="left.secondary[0]">
                          <option v-for="source in sources">{{source}}</option>
                        </select>
                      </div>
                    </div>
                    <div class="control" v-if="left.secondary[0]=='HTTP'">
                      <a class="button is-primary" v-on:click="showModalHTTP(left.secondary,4)">
                        <span class="icon">
                          <i class="icon-gear"></i>
                        </span>
                        <span>Config</span>
                      </a>
                    </div>
                    <div class="control" v-else-if="left.secondary[0]=='MQTT'">
                      <a class="button is-primary" v-on:click="showModalMQTT(left.secondary,4)">
                        <span class="icon">
                          <i class="icon-gear"></i>
                        </span>
                        <span>Config</span>
                      </a>
                    </div>
                  </div>
                </div>
              </div>
              <template v-if="left.secondary[0]!='none'">
                <div class="field is-horizontal" v-for="(value,idx) in left.secondary[1]">
                  <div class="field-label">
                    <label class="label" v-if="idx==0">Lowest value</label>
                    <label class="label" v-else-if="idx==left.secondary[1].length-1">Highest value</label>
                    <label class="label" v-else="idx==0"></label>
                  </div>
                  <div class="field-body">
                    <div class="field has-addons">
                      <div class="control" v-if="idx!=left.secondary[1].length-1">
                        <a class="button is-success" v-on:click="left.secondary[1].splice(idx+1,0,(left.secondary[1][idx]+left.secondary[1][idx+1])/2); left.secondary[2].splice(idx+1,0,[255,255,255]); ">
                          <span class="icon">
                            <i class="icon-plus"></i>
                          </span>
                        </a>
                      </div>
                      <div class="control" v-if="idx!=0 && idx!=left.secondary[1].length-1">
                        <a class="button is-danger" v-on:click="left.secondary[1].splice(idx,1); left.secondary[2].splice(idx,1); ">
                          <span class="icon">
                            <i class="icon-remove"></i>
                          </span>
                        </a>
                      </div>
                      <div class="control is-expanded">
                        <input class="input" type="number" step="any" placeholder="Upper bound" v-model.number="left.secondary[1][idx]">
                      </div>
                      <div class="control" v-if="left.secondary[0]!='none'">
                        <a class="button" :style="getBackgroundStyle(left.secondary[2][idx])" v-on:click="showModalColor(left.secondary[2][idx])">
                          <span class="icon">
                            <i class="icon-eyedropper"></i>
                          </span>
                        </a>
                      </div>
                    </div>
                  </div>
                </div>
                <div class="field is-horizontal">
                  <div class="field-label">
                    <label class="label">Update interval</label>
                  </div>
                  <div class="field-body">
                    <div class="field has-addons">
                      <div class="control is-expanded">
                        <input class="input" type="number" step="1" placeholder="Update interval" v-model.number="left.secondary[3]">
                      </div>
                      <div class="control">
                        <a class="button is-static">
                          ms
                        </a>
                      </div>
                    </div>
                  </div>
                </div>
              </template>
            </div>
          </div>
        </div>
      </div>

      <p class="title is-3">Right side</p>
      <div class="columns is-mobile">
        <div class="column is-narrow">
          <svg height="100%" width="10px">
            <rect v-for="i in numelements"
               v-bind:style="ledcolors[numelements+numelements-1-(i-1)]"
               width="100%"
               v-bind:height="100/numelements+'%'"
               x="0"
               v-bind:y="(i-1)*(100/numelements)+'%'" />
          </svg>
        </div>

        <div class="column">
          <div class="columns">
            <div class="column is-half">
              <p class="title is-5">Primary</p>
              <div class="field is-horizontal">
                <div class="field-label">
                  <label class="label">Data source</label>
                </div>
                <div class="field-body">
                  <div class="field has-addons">
                    <div class="control is-expanded">
                      <div class="select is-fullwidth">
                        <select v-model="right.primary[0]">
                          <option v-for="source in sources">{{source}}</option>
                        </select>
                      </div>
                    </div>
                    <div class="control" v-if="right.primary[0]=='HTTP'">
                      <a class="button is-primary" v-on:click="showModalHTTP(right.primary,4)">
                        <span class="icon">
                          <i class="icon-gear"></i>
                        </span>
                        <span>Config</span>
                      </a>
                    </div>
                    <div class="control" v-else-if="right.primary[0]=='MQTT'">
                      <a class="button is-primary" v-on:click="showModalMQTT(right.primary,4)">
                        <span class="icon">
                          <i class="icon-gear"></i>
                        </span>
                        <span>Config</span>
                      </a>
                    </div>
                  </div>
                </div>
              </div>
              <template v-if="right.primary[0]!='none'">
                <div class="field is-horizontal" v-for="(value,idx) in right.primary[1]">
                  <div class="field-label">
                    <label class="label" v-if="idx==0">Lowest value</label>
                    <label class="label" v-else-if="idx==right.primary[1].length-1">Highest value</label>
                    <label class="label" v-else="idx==0"></label>
                  </div>
                  <div class="field-body">
                    <div class="field has-addons">
                      <div class="control" v-if="idx!=right.primary[1].length-1">
                        <a class="button is-success" v-on:click="right.primary[1].splice(idx+1,0,(right.primary[1][idx]+right.primary[1][idx+1])/2); right.primary[2].splice(idx+1,0,[255,255,255]); ">
                          <span class="icon">
                            <i class="icon-plus"></i>
                          </span>
                        </a>
                      </div>
                      <div class="control" v-if="idx!=0 && idx!=right.primary[1].length-1">
                        <a class="button is-danger" v-on:click="right.primary[1].splice(idx,1); right.primary[2].splice(idx,1); ">
                          <span class="icon">
                            <i class="icon-remove"></i>
                          </span>
                        </a>
                      </div>
                      <div class="control is-expanded">
                        <input class="input" type="number" step="any" placeholder="Upper bound" v-model.number="right.primary[1][idx]">
                      </div>
                      <div class="control" v-if="right.secondary[0]=='none'">
                        <a class="button" :style="getBackgroundStyle(right.primary[2][idx])" v-on:click="showModalColor(right.primary[2][idx])">
                          <span class="icon">
                            <i class="icon-eyedropper"></i>
                          </span>
                        </a>
                      </div>
                    </div>
                  </div>
                </div>
                <div class="field is-horizontal">
                  <div class="field-label">
                    <label class="label">Update interval</label>
                  </div>
                  <div class="field-body">
                    <div class="field has-addons">
                      <div class="control is-expanded">
                        <input class="input" type="number" step="1" placeholder="Update interval" v-model.number="right.primary[3]">
                      </div>
                      <div class="control">
                        <a class="button is-static">
                          ms
                        </a>
                      </div>
                    </div>
                  </div>
                </div>
              </template>
            </div>

            <div class="column">
              <p class="title is-5">Secondary</p>
              <div class="field is-horizontal">
                <div class="field-label">
                  <label class="label">Data source</label>
                </div>
                <div class="field-body">
                  <div class="field has-addons">
                    <div class="control is-expanded">
                      <div class="select is-fullwidth">
                        <select v-model="right.secondary[0]">
                          <option v-for="source in sources">{{source}}</option>
                        </select>
                      </div>
                    </div>
                    <div class="control" v-if="right.secondary[0]=='HTTP'">
                      <a class="button is-primary" v-on:click="showModalHTTP(right.secondary,4)">
                        <span class="icon">
                          <i class="icon-gear"></i>
                        </span>
                        <span>Config</span>
                      </a>
                    </div>
                    <div class="control" v-else-if="right.secondary[0]=='MQTT'">
                      <a class="button is-primary" v-on:click="showModalMQTT(right.secondary,4)">
                        <span class="icon">
                          <i class="icon-gear"></i>
                        </span>
                        <span>Config</span>
                      </a>
                    </div>
                  </div>
                </div>
              </div>
              <template v-if="right.secondary[0]!='none'">
                <div class="field is-horizontal" v-for="(value,idx) in right.secondary[1]">
                  <div class="field-label">
                    <label class="label" v-if="idx==0">Lowest value</label>
                    <label class="label" v-else-if="idx==right.secondary[1].length-1">Highest value</label>
                    <label class="label" v-else="idx==0"></label>
                  </div>
                  <div class="field-body">
                    <div class="field has-addons">
                      <div class="control" v-if="idx!=right.secondary[1].length-1">
                        <a class="button is-success" v-on:click="right.secondary[1].splice(idx+1,0,(right.secondary[1][idx]+right.secondary[1][idx+1])/2); right.secondary[2].splice(idx+1,0,[255,255,255]); ">
                          <span class="icon">
                            <i class="icon-plus"></i>
                          </span>
                        </a>
                      </div>
                      <div class="control" v-if="idx!=0 && idx!=right.secondary[1].length-1">
                        <a class="button is-danger" v-on:click="right.secondary[1].splice(idx,1); right.secondary[2].splice(idx,1); ">
                          <span class="icon">
                            <i class="icon-remove"></i>
                          </span>
                        </a>
                      </div>
                      <div class="control is-expanded">
                        <input class="input" type="number" step="any" placeholder="Upper bound" v-model.number="right.secondary[1][idx]">
                      </div>
                      <div class="control" v-if="right.secondary[0]!='none'">
                        <a class="button" :style="getBackgroundStyle(right.secondary[2][idx])" v-on:click="showModalColor(right.secondary[2][idx])">
                          <span class="icon">
                            <i class="icon-eyedropper"></i>
                          </span>
                        </a>
                      </div>
                    </div>
                  </div>
                </div>
                <div class="field is-horizontal">
                  <div class="field-label">
                    <label class="label">Update interval</label>
                  </div>
                  <div class="field-body">
                    <div class="field has-addons">
                      <div class="control is-expanded">
                        <input class="input" type="number" step="1" placeholder="Update interval" v-model.number="right.secondary[3]">
                      </div>
                      <div class="control">
                        <a class="button is-static">
                          ms
                        </a>
                      </div>
                    </div>
                  </div>
                </div>
              </template>
            </div>
          </div>
        </div>
      </div>

      <div class="field-body">
        <div class="field">
          <div class="control">
            <button class="button is-primary" v-bind:class="{'is-loading': commitWorking}" :disabled="commitWorking" type="submit">
              <span class="icon">
                <i class="icon-save"></i>
              </span>
              <span>Save</span>
            </button>
          </div>
        </div>
      </div>
    </form>

    <div class="modal is-active" v-if="colorModalShown">
      <div class="modal-background"></div>
      <div class="modal-content">
        <ModalColor :color="modalcolor" dialogTitle="Select Color" v-on:cancel="colorModalShown=false" v-on:commit="applyColor"></ModalColor>
      </div>
    </div>
    <div class="modal is-active" v-if="httpModalShown">
      <div class="modal-background"></div>
      <div class="modal-content">
        <ModalHTTP :settingsarray="modalarray" dialogTitle="Edit HTTP settings" v-on:cancel="httpModalShown=false" v-on:commit="applyHTTP"></ModalHTTP>
      </div>
    </div>
    <div class="modal is-active" v-if="mqttModalShown">
      <div class="modal-background"></div>
      <div class="modal-content">
        <ModalMQTT :settingsarray="modalarray" dialogTitle="Edit MQTT settings" v-on:cancel="mqttModalShown=false" v-on:commit="applyMQTT"></ModalMQTT>
      </div>
    </div>
  </section>
  <BottomFooter></BottomFooter>
</div>
</template>

<script>
import Vue from 'vue'
import ModalColor from '../components/ModalColor.vue'
import ModalHTTP from '../components/ModalHTTP.vue'
import ModalMQTT from '../components/ModalMQTT.vue'
import WebEvents from '../plugins/webevents'

import {
  HTTP
} from '../plugins/axios'

export default {
  name: 'MainBookmarks',
  components: {
    ModalColor,
    ModalHTTP,
    ModalMQTT
  },
  computed: {
  },
  data() {
    return {
      numelements: 12,
      colorModalShown: false,
      httpModalShown: false,
      mqttModalShown: false,
      modalcolor: [],
      modalarray: [],
      targetarray: [],
      targetidx: 0,
      commitWorking: false,
      ledcolors: Array(38).fill({fill: "#ffffff", filter: "url('#mySVGfilter')"}),
      sources: [],
      sourcesbrightness: [],
      brightness: [],
      left: {primary: ["",[],[],0], secondary: ["",[],[],0]},
      right: {primary: ["",[],[],0], secondary: ["",[],[],0]},
      sensorvalues: {temperature: 0, humidity: 0, eco2: 0, tvoc: 0}
    }
  },
  methods: {
    // request a color gradient from the ESP8266 based on two colors. not used anymore currently as there might also be gradients
    // with more than two colors.
    getColors: function(color1, color2, myarray, startindex) {
      HTTP.get('/api/colors?color1='+color1[0].toString(16).padStart(2,'0')+color1[1].toString(16).padStart(2,'0')+color1[2].toString(16).padStart(2,'0')+'&color2='+color2[0].toString(16).padStart(2,'0')+color2[1].toString(16).padStart(2,'0')+color2[2].toString(16).padStart(2,'0')).then((response) => {
        if (response.status == 200 && (response.headers["content-type"].includes("application/json") || response.headers["content-type"].includes("text/json"))) {
          for(var i=startindex; i<startindex+response.data.length; i++) {
            myarray.splice(i,1,{fill: response.data[i-startindex]});
          }
        }
      }).catch((e) => {
        this.$toast.open({
          message: 'Couldn\'t get colors from ESP.',
          type: 'is-danger',
          position: 'is-bottom'
        });
      });
    },
    // convert a JavaScript array that contains a color value to an object compatible with CSS
    getBackgroundStyle: function(val) {
      if(val.constructor===Array && val.length==3) {
        return {background: "#"+val[0].toString(16).padStart(2,'0')+val[1].toString(16).padStart(2,'0')+val[2].toString(16).padStart(2,'0'),
                color: "#"+(val[0]+val[1]+val[2]<384 ? "ffffff" : "000000")};
      } else {
        return {background: "#000000"};
      }
    },
    // shows a modal to modify a color value
    showModalColor: function(val) {
      this.modalcolor=val;
      this.colorModalShown=true;
    },
    // is called when the color modal finishes and applies the color changes
    applyColor: function(retval,loadingComponent) {
      this.colorModalShown=false;
      this.modalcolor[0]=retval.r;
      this.modalcolor[1]=retval.g;
      this.modalcolor[2]=retval.b;
      //this.getColors(this.left.secondary[2],this.left.secondary[4],this.ledcolors,0);
      //this.getColors(this.right.secondary[2],this.right.secondary[4],this.ledcolors,12);
      loadingComponent.close();
    },
    // shows a modal to modify HTTP request settings
    showModalHTTP: function(array, arrayindex) {
      if(!(array[arrayindex]===undefined) && array[arrayindex][0]==="HTTP") {
        this.targetarray=array;
        this.targetidx=arrayindex;
        this.modalarray=array[arrayindex].slice(1);
      } else {
        array[arrayindex]=[];
        this.targetarray=array;
        this.targetidx=arrayindex;
        this.modalarray=[];
      }
      this.httpModalShown=true;
    },
    // is called when the HTTP modal finishes and applies the setting changes
    applyHTTP: function(retval,loadingComponent) {
      this.httpModalShown=false;
      this.targetarray[this.targetidx][0]="HTTP";
      for(var i=0; i<retval.length; i++) {
        this.targetarray[this.targetidx][1+i]=retval[i];
      }
      loadingComponent.close();
    },
    // shows a modal to modify MQTT request settings
    showModalMQTT: function(array, arrayindex) {
      if(!(array[arrayindex]===undefined) && array[arrayindex][0]==="MQTT") {
        this.targetarray=array;
        this.targetidx=arrayindex;
        this.modalarray=array[arrayindex].slice(1);
      } else {
        array[arrayindex]=[];
        this.targetarray=array;
        this.targetidx=arrayindex;
        this.modalarray=[];
      }
      this.mqttModalShown=true;
    },
    // is called when the MQTT modal finishes and applies the setting changes
    applyMQTT: function(retval,loadingComponent) {
      this.mqttModalShown=false;
      this.targetarray[this.targetidx][0]="MQTT";
      for(var i=0; i<retval.length; i++) {
        this.targetarray[this.targetidx][1+i]=retval[i];
      }
      loadingComponent.close();
    },
    // sends all changed settings to the ESP8266
    saveSettings: function() {
      this.commitWorking=true;
      const loadingComponent = this.$loading.open();

      var newSettings={brightness: this.brightness,
                       left: {primary: this.left.primary,
                              secondary: this.left.secondary},
                       right: {primary: this.right.primary,
                               secondary: this.right.secondary}};

      HTTP.put('/api/displaysettings',newSettings).then((response) => {
        this.$toast.open({
          message: 'Display settings saved successfully.',
          type: 'is-success',
          position: 'is-bottom'
        });
      }).catch((ex) => {
        this.$toast.open({
          message: 'Error during saving of display settings.',
          type: 'is-danger',
          position: 'is-bottom'
        });
      }).then(() => {
          this.commitWorking=false;
          loadingComponent.close();
      })
    },
    // makes sure that the brightness settings array has the correct length
    brightnesssourcechanged: function() {
      if(this.brightness[0]==="fixed") {
        this.brightness.length=2;
        this.brightness[1]=50;
      } else if(this.brightness[0]==="MQTT") {
        this.brightness.length=7;
        this.brightness[1]="";
        this.brightness[2]=0;
        this.brightness[3]=0;
        this.brightness[4]=0;
        this.brightness[5]=0;
        this.brightness[6]=false;
      }
    }
  },
  // on create, register a new server-sent events listener
  created: function() {
    if(WebEvents.sourceAvailable()) {
      this.$eventsource.addEventListener('SensorValue', e => {
        try {
          var newevent=JSON.parse(e.data);
          for (const [key, value] of Object.entries(newevent)) {
            if(key in this.sensorvalues) {
              this.sensorvalues[key]=value;
            }
          }
        } catch (err) {
        }
      }, false);
    }

    // get the current settings from the ESP8266.
    // execution is delayed by 2.5 seconds, so that the ESP is not overburdened with concurrent request on page refresh
    window.setTimeout(()=>{
      HTTP.get('/api/displaysettings').then((response) => {
        if (response.status == 200 && (response.headers["content-type"].includes("application/json") || response.headers["content-type"].includes("text/json"))) {
          if("brightness" in response.data) {
            this.brightness=response.data.brightness;
          } else {
          }
          if("sourcesbrightness" in response.data) {
            this.sourcesbrightness=response.data.sourcesbrightness;
          } else {
          }
          if("sources" in response.data) {
            this.sources=response.data.sources;
          } else {
          }
          if("left" in response.data) {
            this.left=response.data.left;
            //this.getColors(response.data.left.secondary[2],response.data.left.secondary[4],this.ledcolors,0);
          } else {
          }
          if("right" in response.data) {
            this.right=response.data.right;
            //this.getColors(response.data.right.secondary[2],response.data.right.secondary[4],this.ledcolors,12);
          } else {
          }
          if("colorleft" in response.data) {
            for(var i=0; i<0+response.data.colorleft.length; i++) {
              this.ledcolors.splice(i,1,{fill: response.data.colorleft[i-0]});
            }
          }
          if("colorright" in response.data) {
            for(var i=12; i<12+response.data.colorright.length; i++) {
              this.ledcolors.splice(i,1,{fill: response.data.colorright[i-12]});
            }
          }
        } else {
        }
      }).catch((e) => {
        this.$toast.open({
          message: 'Couldn\'t get display settings from ESP.',
          type: 'is-danger',
          position: 'is-bottom'
        });
      });
    },2500);
  }
}
</script>

<style scoped>
  input[type=range].slider.is-white-gradient::-moz-range-track {
    background-image: linear-gradient(to right, black , white);
  }

  input[type=range].slider.is-white-gradient::-webkit-slider-runnable-track {
    background-image: linear-gradient(to right, black , white);
  }

  input[type=range].slider.is-white-gradient::-ms-track {
    background-image: linear-gradient(to right, black , white) !important;
  }

  input[type=range].slider.is-white-gradient::-ms-fill-lower {
    background-image: linear-gradient(to right, black , white);
  }
</style>
