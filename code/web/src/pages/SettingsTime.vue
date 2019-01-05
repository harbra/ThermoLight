<template>
<div id="app" class="is-marginless">
  <TopNav title="Settings - Time" subtitle="Time settings of the ThermoLight can be configured here" menutitle="Overview" menutitleicon="thermometer"
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
              <input type="checkbox" v-model="SettingsTime.useNTP"> Use NTP?
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">NTP Server 1</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input :disabled="!SettingsTime.useNTP" class="input" type="text" placeholder="NTP Server 1" v-model="SettingsTime.NTPServer1">
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">NTP Server 2</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input :disabled="!SettingsTime.useNTP" class="input" type="text" placeholder="NTP Server 2" v-model="SettingsTime.NTPServer2">
            </p>
          </div>
        </div>
      </div>


      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">NTP Server 3</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input :disabled="!SettingsTime.useNTP" class="input" type="text" placeholder="NTP Server 3" v-model="SettingsTime.NTPServer3">
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
      SettingsTime: { useNTP: false,
                      NTPServer1:"",
                      NTPServer2:"",
                      NTPServer3:""
                    },
      commitWorking: false
    }
  },
  computed: {
  },
  methods: {
    // save the changes to the time settings
    commitChanges: function() {
      this.commitWorking=true;
      const loadingComponent = this.$loading.open();

      var newSettings={time: {ntpen: this.SettingsTime.useNTP,
                              ntps1: this.SettingsTime.NTPServer1,
                              ntps2: this.SettingsTime.NTPServer2,
                              ntps3: this.SettingsTime.NTPServer3}};

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
          this.SettingsTime.useNTP=response.data["time"]["ntpen"];
          this.SettingsTime.NTPServer1=response.data["time"]["ntps1"];
          this.SettingsTime.NTPServer2=response.data["time"]["ntps2"];
          this.SettingsTime.NTPServer3=response.data["time"]["ntps3"];
        } else {
          this.SettingsTime={};
        }
      }).catch((e) =>
      {
        this.SettingsTime={};
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
