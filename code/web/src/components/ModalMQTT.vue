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
        <label class="label">Topic</label>
      </div>
      <div class="field-body">
        <div class="field">
          <p class="control">
            <input class="input" type="text" placeholder="/topicparta/topicpartb" v-model="topic">
          </p>
        </div>
      </div>
    </div>
    <div class="field is-horizontal">
      <div class="field-label is-normal">
        <label class="label">Scaling</label>
      </div>
      <div class="field-body">
        <div class="field">
          <input class="input" type="number" step="any" placeholder="1" v-model.number="multiplicator">
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
          <input class="input" type="number" step="any" placeholder="0" v-model.number="summand">
        </div>
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
      topic: (this.settingsarray.length>0 ? this.settingsarray[0] : ''),
      multiplicator: (this.settingsarray.length>1 ? this.settingsarray[1] : 1),
      summand: (this.settingsarray.length>2 ? this.settingsarray[2] : 0)
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

      var retval=[this.topic, parseFloat(this.multiplicator), parseFloat(this.summand)];

      this.$emit('commit',retval,loadingComponent);
    }
  }
}
</script>

<style scoped>
</style>
