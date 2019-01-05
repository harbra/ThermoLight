<!--
This vue.js file is the main settings page for network settings.
-->
<template>
<div id="app" class="is-marginless">
  <TopNav title="Settings - MQTT" subtitle="MQTT settings of the ThermoLight can be configured here" menutitle="Overview" menutitleicon="thermometer"
  :sublinks="[{linkto:'/settings/network', linktext:'Network'},
              {linkto:'/settings/time', linktext:'Time'},
              {linkto:'/settings/mqtt', linktext:'MQTT'},
              {linkto:'/settings/backup', linktext:'Backup/Restore'}]"></TopNav>
  <section class="section">
    <form v-on:submit.prevent="commitChanges()">
    <div class="container">
      <div class="field is-horizontal is-narrow">
        <div class="field-label">
          <label class="label">Connection</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input type="checkbox" v-model="SettingsMQTT.useMQTT"> Use MQTT?
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">Host &amp; Port</label>
        </div>
        <div class="field-body">
          <div class="field has-addons">
            <p class="control is-expanded">
              <input required :disabled="!SettingsMQTT.useMQTT" class="input" type="text" placeholder="Host" v-model="SettingsMQTT.MQTTHost">
            </p>
            <p class="control">
              <input required :disabled="!SettingsMQTT.useMQTT" class="input" type="number" placeholder="Port" v-model="SettingsMQTT.MQTTPort">
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">Username</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input :disabled="!SettingsMQTT.useMQTT" class="input" type="text" placeholder="MQTT username" v-model="SettingsMQTT.MQTTUser">
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">Password</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input :disabled="!SettingsMQTT.useMQTT" class="input" type="password" placeholder="Password for MQTT server" v-model="SettingsMQTT.MQTTPassword" v-on:change="SettingsMQTT.MQTTPassword_changed=true">
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label">
          <!-- Left empty for spacing -->
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
      </div>
    </div>
    </form>
  </section>
  <BottomFooter></BottomFooter>
</div>
</template>

<script>
import {HTTP} from '../plugins/axios'

export default {
  name: 'SettingsMQTT',
  components: {
  },
  data() {
    return {
      SettingsMQTT: { useMQTT: false,
                      MQTTHost:"",
                      MQTTPort: false,
                      MQTTUser:"",
                      MQTTPassword:"",
                      MQTTPassword_changed: false
                    },
      commitWorking: false
    }
  },
  computed: {
  },
  methods: {
    // save the changes to the MQTT settings
    commitChanges: function() {
      this.commitWorking=true;
      const loadingComponent = this.$loading.open();

      var newSettings={mqtt: {connect: this.SettingsMQTT.useMQTT,
                              host: this.SettingsMQTT.MQTTHost,
                              port: this.SettingsMQTT.MQTTPort,
                              user: this.SettingsMQTT.MQTTUser}};
      if (this.SettingsMQTT.MQTTPassword_changed) {
        newSettings.mqtt.pass=this.SettingsMQTT.MQTTPassword;
      }

      HTTP.post('/api/settings', newSettings
    ).then((response) => {
      this.$toast.open({
                    message: 'Settings saved successfully.',
                    type: 'is-success',
                    position: 'is-bottom'
                })
    }).catch((e) => {
      this.$toast.open({
                    message: 'Failed to save settings.',
                    type: 'is-danger',
                    position: 'is-bottom'
                })
    }).then(() => {
        this.commitWorking=false;
        loadingComponent.close();
      })
    }
  },
  // when the component is created, get the settings from the ESP and handle display.
  // passwords are not transmitted back by the ESP, but a dummy value is inserted
  created: function() {
    HTTP.get('/api/settings?type=*').then((response) =>
      {
        if(response.status==200 && (response.headers["content-type"].includes("application/json") || response.headers["content-type"].includes("text/json")))
        {
          this.SettingsMQTT.useMQTT=response.data["mqtt"]["connect"];
          this.SettingsMQTT.MQTTHost=response.data["mqtt"]["host"];
          this.SettingsMQTT.MQTTPort=response.data["mqtt"]["port"];
          this.SettingsMQTT.MQTTUser=response.data["mqtt"]["user"];
          this.SettingsMQTT.MQTTPassword="XXXXXXXX";
          this.SettingsMQTT.MQTTPassword_changed=false;
        } else {
          this.SettingsMQTT={};
        }
      }).catch((e) =>
      {
        this.SettingsMQTT={};
        this.$toast.open({
                      message: 'Couldn\'t get settings from ESP.',
                      type: 'is-danger',
                      position: 'is-bottom'
                  });
      })
  }
}
</script>

<style scoped>

</style>
