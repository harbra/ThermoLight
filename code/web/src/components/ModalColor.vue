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
    <div class="box" :style="selected_color">
    </div>

    <div class="field is-horizontal">
      <div class="field-label is-normal">
        <label class="label">R</label>
      </div>
      <div class="field-body">
        <div class="field is-grouped">
          <p class="control is-expanded">
            <input class="slider is-fullwidth is-red-gradient" step="1" min="0" max="255" v-model.number="color_r" type="range" />
          </p>
          <p class="control">
            <input class="input" type="number" min="0" max="255" v-model.number="color_r" style="width: 70px" />
          </p>
        </div>
      </div>
    </div>

    <div class="field is-horizontal">
      <div class="field-label is-normal">
        <label class="label">G</label>
      </div>
      <div class="field-body">
        <div class="field is-grouped">
          <p class="control is-expanded">
            <input class="slider is-fullwidth is-green-gradient" step="1" min="0" max="255" v-model.number="color_g" type="range" />
          </p>
          <p class="control">
            <input class="input" type="number" min="0" max="255" v-model.number="color_g" style="width: 70px" />
          </p>
        </div>
      </div>
    </div>

    <div class="field is-horizontal">
      <div class="field-label is-normal">
        <label class="label">B</label>
      </div>
      <div class="field-body">
        <div class="field is-grouped">
          <p class="control is-expanded">
            <input class="slider is-fullwidth is-blue-gradient" step="1" min="0" max="255" v-model.number="color_b" type="range" />
          </p>
          <p class="control">
            <input class="input" type="number" min="0" max="255" v-model.number="color_b" style="width: 70px" />
          </p>
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
export default {
  name: 'ModalColor',
  computed: {
    // convert this.color_r, this.color_g and this.color_b to a web-friendly hex string, i.e. #ff0000 for red
    selected_color: function() {
      return {background: "#"+parseInt(this.color_r).toString(16).padStart(2,'0')+parseInt(this.color_g).toString(16).padStart(2,'0')+parseInt(this.color_b).toString(16).padStart(2,'0')};
    }
  },
  data() {
    return {
      color_r: (this.color.constructor === String ? parseInt(this.color.substring(1,3),16) : this.color[0]),
      color_g: (this.color.constructor === String ? parseInt(this.color.substring(3,5),16) : this.color[1]),
      color_b: (this.color.constructor === String ? parseInt(this.color.substring(5,7),16) : this.color[2])
    }
  },
  // watch the color variables, in order to limit the values to between 0 and 255
  watch: {
    color_r: function(value) {
      this.color_r=Math.min(255,Math.max(0,value));
    },
    color_g: function(value) {
      this.color_g=Math.min(255,Math.max(0,value));
    },
    color_b: function(value) {
      this.color_b=Math.min(255,Math.max(0,value));
    }
  },
  props: {
    dialogTitle: { type: String,
                   required: true },
    color: {
      validator: function(value) {
        if(value.constructor === String) {
          var strval=String(value);
          if(strval.length!=7) return false;
          if(strval.substr(0,1)!="#") return false;
          if(isNaN(parseInt(strval.substr(1),16))) return false;

          return true;
        } else if(value.constructor === Array) {
          if(value.length!=3) return false;
          for(var i=0; i<3; i++) {
            if(isNaN(parseInt(value[i]))) return false;
          }

          return true;
        }

        return false;
      }
    }
  },
  methods: {
    cancelModal: function() {
      this.$emit('cancel');
    },
    commitModal: function() {
      const loadingComponent = this.$loading.open();

      var retval={
          r: Math.min(255,Math.max(0,parseInt(this.color_r))),
          g: Math.min(255,Math.max(0,parseInt(this.color_g))),
          b: Math.min(255,Math.max(0,parseInt(this.color_b)))
        }

      this.$emit('commit',retval,loadingComponent);
    }
  }
}
</script>

<style scoped>
  input[type=range].slider.is-red-gradient::-moz-range-track {
    background-image: linear-gradient(to right, black , red);
  }

  input[type=range].slider.is-red-gradient::-webkit-slider-runnable-track {
    background-image: linear-gradient(to right, black , red);
  }

  input[type=range].slider.is-red-gradient::-ms-track {
    background-image: linear-gradient(to right, black , red) !important;
  }

  input[type=range].slider.is-red-gradient::-ms-fill-lower {
    background-image: linear-gradient(to right, black , red);
  }

  input[type=range].slider.is-green-gradient::-moz-range-track {
    background-image: linear-gradient(to right, black , green);
  }

  input[type=range].slider.is-green-gradient::-webkit-slider-runnable-track {
    background-image: linear-gradient(to right, black , green);
  }

  input[type=range].slider.is-green-gradient::-ms-track {
    background-image: linear-gradient(to right, black , green) !important;
  }

  input[type=range].slider.is-green-gradient::-ms-fill-lower {
    background-image: linear-gradient(to right, black , green);
  }

  input[type=range].slider.is-blue-gradient::-moz-range-track {
    background-image: linear-gradient(to right, black , blue);
  }

  input[type=range].slider.is-blue-gradient::-webkit-slider-runnable-track {
    background-image: linear-gradient(to right, black , blue);
  }

  input[type=range].slider.is-blue-gradient::-ms-track {
    background-image: linear-gradient(to right, black , blue) !important;
  }

  input[type=range].slider.is-blue-gradient::-ms-fill-lower {
    background-image: linear-gradient(to right, black , blue);
  }
</style>
