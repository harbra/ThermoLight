<!--
This vue.js file contains a modal that can be used to create and modify names.
-->
<template>
<form v-on:submit.prevent="commitModal()">
<div class="card">
  <div class="card-header">
    <div class="card-header-title">
      {{dialogTitle}}
    </div>
  </div>
  <div class="card-content">
    <div class="field is-horizontal">
      <div class="field-label is-normal">
        <label class="label">URL</label>
      </div>
      <div class="field-body">
        <div class="field">
          <p class="control">
            <input class="input" type="text" placeholder="http://test.com/value" v-model="url" v-on:change="testresponse=''">
          </p>
        </div>
      </div>
    </div>
    <div class="field is-horizontal">
      <div class="field-label is-normal">
        <label class="label">Port</label>
      </div>
      <div class="field-body">
        <div class="field">
          <p class="control">
            <input class="input" type="number" min="0" step="1" placeholder="80" v-model.number="port" v-on:change="testresponse=''">
          </p>
        </div>
      </div>
    </div>
    <div class="field is-horizontal">
      <div class="field-label is-normal">
        <label class="label">JSON</label>
      </div>
      <div class="field-body">
        <template v-for="(tag, idx) in jsonlocation">
          <span class="icon" style="height: 100%; margin-left: -12px;" v-if="idx>0">
            <i class="icon-chevron-right"></i>
          </span>
          <div class="field">
            <input class="input" type="text" placeholder="tag" v-model="jsonlocation[idx]" v-on:change="testresponse=''">
          </div>
        </template>
        <a class="button is-warning is-rounded" v-on:click="jsonlocation.pop()" :disabled="jsonlocation.length<=1">
          <span class="icon">
            <i class="icon-remove"></i>
          </span>
        </a>
        <a class="button is-primary is-rounded" v-on:click.stop="jsonlocation.push('')">
          <span class="icon">
            <i class="icon-plus"></i>
          </span>
        </a>
      </div>
    </div>
    <div class="field is-horizontal">
      <div class="field-label is-normal">
        <label class="label">Scaling</label>
      </div>
      <div class="field-body">
        <div class="field">
          <input class="input" type="number" step="any" placeholder="1" v-model.number="multiplicator" v-on:change="testresponse=''">
        </div>
        <span class="icon" style="height: 100%; margin-left: -12px;">
          <i class="icon-times-circle"></i>
        </span>
        <span class="icon has-text-info" style="height: 100%;">
          <i class="icon-hashtag"></i>
        </span>
        <span class="icon" style="height: 100%;">
          <i class="icon-plus-circle"></i>
        </span>
        <div class="field">
          <input class="input" type="number" step="any" placeholder="0" v-model.number="summand" v-on:change="testresponse=''">
        </div>
      </div>
    </div>
    <div class="field is-horizontal">
      <div class="field-label">
        <!-- Left empty for spacing -->
      </div>
      <div class="field-body">
        <a class="button" v-on:click="testConnection" :disabled="url==='' || port===''">
          <span class="icon">
            <i class="icon-globe"></i>
          </span>
          <span>Test</span>
        </a>
      </div>
    </div>
    <div class="field is-horizontal">
      <div class="field-label">
        <!-- Left empty for spacing -->
      </div>
      <div class="field-body">
        <textarea class="textarea is-small" type="text" readonly v-model="testresponse" :disabled="testresponse===''" style="font-family:monospace;"
></textarea>
      </div>
    </div>

    <div class="card-footer">
      <a class="card-footer-item button is-primary" v-on:click="commitModal()">
        <span class="icon">
          <i class="icon-check"></i>
        </span>
        <span>Ok</span>
      </a>
      <a class="card-footer-item button is-danger" v-on:click="cancelModal()">
        <span class="icon">
          <i class="icon-remove"></i>
        </span>
        <span>Cancel</span>
      </a>
    </div>
  </div>
</div>
</form>
</template>

<script>
import {
  HTTP
} from '../plugins/axios'

export default {
  name: 'ModalHTTP',
  computed: {
  },
  data() {
    return {
      url: (this.settingsarray.length>0 ? this.settingsarray[0] : ''),
      port: (this.settingsarray.length>1 ? this.settingsarray[1] : 80),
      jsonlocation: (this.settingsarray.length>2 ? this.settingsarray[2] : ['']),
      multiplicator: (this.settingsarray.length>3 ? this.settingsarray[3] : 1),
      summand: (this.settingsarray.length>4 ? this.settingsarray[4] : 0),
      testresponse: ""
    }
  },
  watch: {
  },
  props: {
    dialogTitle: { type: String,
                   required: true },
    settingsarray: { type: Array,
                     required: true }
  },
  methods: {
    cancelModal: function() {
      this.$emit('cancel');
    },
    commitModal: function() {
      const loadingComponent = this.$loading.open();

      var retval=[this.url, parseInt(this.port), this.jsonlocation, parseFloat(this.multiplicator), parseFloat(this.summand)];

      this.$emit('commit',retval,loadingComponent);
    },
    // Try to use the settings in the form to connect to a web endpoint and see if the JSON parsing, etc. yields the correct
    // value. This is run from the browser and not directly from the ESP8266, so results might vary when using the settings
    // on the ThermoLight itself.
    testConnection: function() {
      var pathstart=this.url.indexOf("/",this.url.indexOf("//")+2);
      if(pathstart!=-1) {
        var baseurl=this.url.substring(0,pathstart);
        var path=this.url.substring(pathstart);
      } else {
        var baseurl=this.url;
        var path="";
      }

      this.testresponse="HTTP GET "+baseurl+":"+this.port+path+"\n";
      HTTP.get(baseurl+":"+this.port+path).then((response) => {
        if(response.data.constructor!=Object) {
          this.testresponse+="No valid JSON received";
          return;
        }

        var currentlevel=response.data;
        var error=-1;
        for(var i=0; i<this.jsonlocation.length; i++) {
          if(this.jsonlocation[i] in currentlevel) {
            currentlevel=currentlevel[this.jsonlocation[i]];
          } else {
            error=i;
            break;
          }
        }

        if(error==-1) {
          this.testresponse+="JSON keys found\n";
          if(!isNaN(parseFloat(currentlevel))) {
            this.testresponse+="Response: "+currentlevel+"\n";
            this.testresponse+="Calculated value: "+(parseFloat(currentlevel)*parseFloat(this.multiplicator)+parseFloat(this.summand));
          } else {
            this.testresponse+="Value not numeric, thus calculation failed.\n";
            this.testresponse+="Response: "+JSON.stringify(currentlevel);
          }
        } else {
          this.testresponse+="Couldn't resolve JSON keys. Error on level "+error+" ("+this.jsonlocation[error]+")\n";
          this.testresponse+="Current remaining response: "+JSON.stringify(currentlevel);
        }
      }).catch((e) => {
        if(e.response) {
          this.testresponse+="Error "+e.response.status+" "+e.response.statusText+"\n";
          this.testresponse+="Response: "+JSON.stringify(e.response.data);
        } else {
          console.log(e);
          this.testresponse+="Unknown error";
        }
      });
    }
  }
}
</script>

<style scoped>
</style>
