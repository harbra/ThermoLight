<!--
This vue.js file is the main settings page for backing up and restoring settings.
-->
<template>
<div id="app" class="is-marginless">
  <TopNav title="Settings - Backup/Restore" subtitle="Backup settings and restore a previously downloaded settings file" menutitle="Overview" menutitleicon="thermometer"
  :sublinks="[{linkto:'/settings/network', linktext:'Network'},
              {linkto:'/settings/time', linktext:'Time'},
              {linkto:'/settings/mqtt', linktext:'MQTT'},
              {linkto:'/settings/backup', linktext:'Backup/Restore'}]"></TopNav>
  <section class="section">
    <div class="container">
      <h1 class="title is-4">Backup</h1>
      <div class="content">
        <div class="content">Please note that sensitive settings files, e.g containing WiFi passwords, are not included in the download.</div>
        <a class="button is-info" href="/api/downloadsettings" download="backup.json" target="_blank">
          <span class="icon">
            <i class="icon-download"></i>
          </span>
          <span>Download settings</span>
        </a>
      </div>

      <h1 class="title is-4">OTA</h1>
      <div class="content">
        <template v-if="ota_enabled">
          <a class="button is-warning" v-on:click="disableOTA()" v-bind:class="{'is-loading': ota_working}" :disabled="ota_working">
            <span class="icon">
              <i class="icon-send"></i>
            </span>
            <span>Disable OTA</span>
          </a>
        </template>
        <template v-else>
          <a class="button is-success" v-on:click="enableOTA()" v-bind:class="{'is-loading': ota_working}" :disabled="ota_working">
            <span class="icon">
              <i class="icon-send"></i>
            </span>
            <span>Enable OTA</span>
          </a>
        </template>
        <div v-if="ota_status==='active'">
          Processing OTA update. Refresh page once update is finished.
        </div>
      </div>

      <h1 class="title is-4">Restore</h1>
      <form enctype="multipart/form-data" novalidate>
        <div class="file has-name is-fullwidth">
          <label class="file-label">
            <input class="file-input" type="file" name="fileUpload" @change="fileSelect($event.target.name, $event.target.files);" />
            <span class="file-cta">
              <span class="icon">
                <i class="icon-upload"></i>
              </span>
              <span class="file-label">
                <span>Upload file</span>
              </span>
            </span>
            <span class="file-name">
              {{uploadFileName}}
            </span>
          </label>
        </div>
      </form>
      <table class="table is-fullwidth" v-if="restoreStatus>=1" style="border: none !important" id="statustable">
        <col width="30%">
        <col width="70%">
        <tr v-if="restoreStatus==1">
          <td>
            Uploading...
          </td>
          <td>
            <progress class="progress" :value="progressValue" max="100">{{progressValue}}%</progress>
          </td>
        </tr>
        <tr v-if="restoreStatus==2">
          <td>
            Restoring...
          </td>
          <td>
            <progress class="progress" :value="restoreNumCurrent" max="restoreNumTotal">{{100/restoreNumTotal*restoreNumCurrent}}%</progress>
          </td>
        </tr>
        <tr v-if="restoreStatus>=2">
          <td>
          </td>
          <td>
            <div class="field is-grouped is-grouped-multiline">
                <div class="control tags has-addons" v-for="key in Object.keys(restoreFiles)">
                  <span class="tag icon animated faa-spin" v-if="restoreFiles[key].status=='started'">
                    <i class="icon-spinner"></i>
                  </span>
                  <span class="tag icon has-text-danger" v-else-if="restoreFiles[key].status=='failed'">
                    <i class="icon-remove"></i>
                  </span>
                  <span class="tag icon has-text-success" v-else-if="restoreFiles[key].status=='success'">
                    <i class="icon-check"></i>
                  </span>
                  <span class="tag is-dark">{{key}}</span>
                </div>
            </div>
          </td>
        </tr>
        <tr v-if="restoreStatus>=2 && restoreWarnings.length>0" v-for="(warning, idx) in restoreWarnings">
          <td>
            <div v-if="idx==0">
              Warnings:
            </div>
          </td>
          <td>
            <span class="icon has-text-warning">
              <i class="icon-warning"></i>
            </span>
            <span>{{warning}}</span>
          </td>
        </tr>
        <tr v-if="restoreStatus==3">
          <td>
          </td>
          <td>
            <span class="icon has-text-success">
              <i class="icon-check"></i>
            </span>
            <strong>Restore succeeded</strong>
          </td>
        </tr>
        <tr v-if="restoreStatus==4">
          <td>
            <div>
              Restore failed:
            </div>
          </td>
          <td>
            <span class="icon has-text-danger">
              <i class="icon-remove"></i>
            </span>
            <span>{{errorMessage}}</span>
          </td>
        </tr>
      </table>
    </div>
  </section>
  <BottomFooter></BottomFooter>
</div>
</template>

<script>
import Vue from 'vue'
import {HTTP} from '../plugins/axios'
import WebEvents from '../plugins/webevents'

export default {
  name: 'SettingsBackupRestore',
  components: {
  },
  data() {
    return {
      uploadFileName: "",
      progressValue: 0,
      restoreStatus: 0,
      errorMessage: "",
      restoreNumCurrent: 0,
      restoreNumTotal: 0,
      restoreFiles: {},
      restoreWarnings: [],
      ota_enabled: false,
      ota_status: "disabled", // disabled, enabled, active
      ota_working: false
    }
  },
  computed: {
  },
  methods: {
    // handles the upload once a file was selected in the file-select dialog.
    fileSelect: function(fieldName, fileList) {
      this.uploadFileName=fileList[0].name;

      this.progressValue=0;
      this.restoreStatus=0;
      this.errorMessage="";
      this.restoreNumCurrent=0;
      this.restoreNumTotal=0;
      this.restoreFiles={};
      this.restoreWarnings=[];

      // creates a FormData object to allow sending the file to the server
      const formData=new FormData();
      if(!fileList.length) return;

      formData.append(fieldName, fileList[0], fileList[0].name);

      this.restoreStatus=1;

      // callback function for axios that is used to show the upload progress in a progress bar
      var config = {
        onUploadProgress: (progressEvent) => {
          this.progressValue = Math.round( (progressEvent.loaded * 100) / progressEvent.total );
        }
      };

      HTTP.post('/api/restoresettings', formData, config).then((response) => {
      }).catch((e) => {
        console.log(e);
        this.$toast.open({
                      message: 'Failed to save settings.',
                      type: 'is-danger',
                      position: 'is-bottom'
                  })
      }).then(() => {
      })
    },
    // disables the over-the-air firmware update listener on the ESP8266
    disableOTA: function() {
      this.ota_working=true;
      HTTP.get('/api/ota/disable').then((response) => {
      }).catch((e) => {
        this.$toast.open({
                      message: 'Failed to enable OTA.',
                      type: 'is-danger',
                      position: 'is-bottom'
                  })
      }).then(() => {
        this.ota_working=false;
      });
    },
    // enables the over-the-air firmware update listener on the ESP8266
    enableOTA: function() {
      this.ota_working=true;
      HTTP.get('/api/ota/enable').then((response) => {
      }).catch((e) => {
        this.$toast.open({
                      message: 'Failed to disable OTA.',
                      type: 'is-danger',
                      position: 'is-bottom'
                  })
      }).then(() => {
        this.ota_working=false;
      });
    }
  },
  // once the component has been created, register a server-sent events listener to listen for the restore progress and display it accordingly
  created: function() {
    if(WebEvents.sourceAvailable()) {
      // the Restore events handle the progress reporting during settings restore
      this.$eventsource.addEventListener('Restore', e => {
        try {
          var data=JSON.parse(e.data);

          if(data.type=="started") this.restoreStatus=2;
          if(data.type=="done") this.restoreStatus=3;
          if(data.type=="failed") {
            this.restoreStatus=4;
            this.errorMessage=data.reason;
          }

          if(data.type=="progress") {
            this.restoreNumCurrent=data.current;
            this.restoreNumTotal=data.total;
          }

          if(data.type=="warning") {
            this.restoreWarnings.push(data.reason);
          }

          if(data.type=="filestatus") {
            if(!(data.filepath in this.restoreFiles)) {
              this.restoreFiles[data.filepath]={};
            }

            if(data.status=="started") {
              this.restoreFiles[data.filepath].status="started";
            } else if(data.status=="failed") {
              this.restoreFiles[data.filepath].status="failed";
              this.restoreFiles[data.filepath].reason=data.reason;
            } else if(data.status=="success") {
              this.restoreFiles[data.filepath].status="success";
            }
          }
        } catch (err) {
          console.log(err);
        }
      }, false);

      // the OTA events handler handles progress updates during OTA firmware upgrades
      this.$eventsource.addEventListener('OTA', e => {
        try {
          var data=JSON.parse(e.data);

          if("status" in data) {
            this.ota_status=data.status;
            if(this.ota_status==="disabled") {
              this.ota_enabled=false
            } else {
              this.ota_enabled=true;
            }
            if(this.ota_status==="active") {
              this.ota_working=true;
            } else {
              this.ota_working=false;
            }
          } else {
            this.ota_status="disabled";
            this.ota_enabled=false;
          }
        } catch (err) {
          console.log(err);
        }
      }, false);
    }

    // checks if the ESP8266 already has OTA functionality enabled
    // this is reflected in the OTA UI button state
    HTTP.get('/api/ota').then((response) => {
      if (response.status == 200 && (response.headers["content-type"].includes("application/json") || response.headers["content-type"].includes("text/json"))) {
        if("enabled" in response.data) {
          this.ota_enabled=response.data.enabled;
        }
      }
    }).catch((e) => {
      console.log(e);
    });
  }
}
</script>

<style scoped>

td {
   border: none !important
}

</style>
