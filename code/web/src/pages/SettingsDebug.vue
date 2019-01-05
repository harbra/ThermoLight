<!--
This vue.js file is the main settings page for backing up and restoring settings.
-->
<template>
<div id="app" class="is-marginless">
  <TopNav title="Settings - Debug" subtitle="Get debug info from the ThermoLight" menutitle="Overview" menutitleicon="thermometer"
  :sublinks="[{linkto:'/settings/network', linktext:'Network'},
              {linkto:'/settings/time', linktext:'Time'},
              {linkto:'/settings/mqtt', linktext:'MQTT'},
              {linkto:'/settings/backup', linktext:'Backup/Restore'},
              {linkto:'/settings/debug', linktext:'Debug'}]"></TopNav>
  <section class="section">
    <div class="container">
      <h1 class="title is-4"><span>Serial</span>
      <a class="button is-primary is-small" v-on:click="enableSerial()" v-if="!serialEnabled">
        <span class="icon">
          <i class="icon-check"></i>
        </span>
        <span>Enable</span>
      </a>
      <a class="button is-warning is-small" v-on:click="disableSerial()" v-else>
        <span class="icon">
          <i class="icon-minus"></i>
        </span>
        <span>Disable</span>
      </a>
      </h1>

      <div class="content">
        <div class="field">
          <div class="control">
            <textarea ref="serialtextarea" class="textarea is-small" type="text" rows="10" readonly v-model="serialData" style="font-family:monospace;"
  ></textarea>
          </div>
        </div>
      </div>
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
  name: 'SettingsDebug',
  components: {
  },
  data() {
    return {
      serialData: "",
      serialEnabled: false
    }
  },
  computed: {
  },
  methods: {
    // makes the ESP stop sending serial events to the webinterface
    disableSerial: function() {
      HTTP.get('/api/serial/disable').then((response) => {
        this.serialEnabled=false;
      }).catch((e) => {
        this.$toast.open({
                      message: 'Failed to disable serial.',
                      type: 'is-danger',
                      position: 'is-bottom'
                  });
      });
    },
    // enables sending serial events to the webinterface
    enableSerial: function() {
      HTTP.get('/api/serial/enable').then((response) => {
        this.serialEnabled=true;
      }).catch((e) => {
        this.$toast.open({
                      message: 'Failed to enable serial.',
                      type: 'is-danger',
                      position: 'is-bottom'
                  });
      });
    }
  },
  // once the component has been created, register a server-sent events listener to listen for the restore progress and display it accordingly
  created: function() {
    if(WebEvents.sourceAvailable()) {
      this.$eventsource.addEventListener('Serial', e => {
        this.serialData+=e.data+"\n";
        try {
          this.$refs["serialtextarea"]["scrollTop"]=this.$refs["serialtextarea"]["scrollHeight"];
        } catch (err) {
        }
      }, false);
    };

    // checks if the ESP8266 already is sending serial events to the webinterface
    // this is reflected in the serial UI button state
    HTTP.get('/api/serial').then((response) => {
      if (response.status == 200 && (response.headers["content-type"].includes("application/json") || response.headers["content-type"].includes("text/json"))) {
        if("enabled" in response.data) {
          this.serialEnabled=response.data.enabled;
        }
      }
    }).catch((e) => {
      console.log(e);
    });
  }
}
</script>

<style scoped>

.textarea {
  background-color: #333333;
  color: #ffffff;
}

</style>
