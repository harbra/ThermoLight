/*jshint esversion: 6 */
// vue-router configuration

import Vue from 'vue';
import Buefy from 'buefy';
import Router from 'vue-router';
//import Test from '../pages/Test.vue';
import SettingsNetwork from '../pages/SettingsNetwork.vue';
import SettingsMQTT from '../pages/SettingsMQTT.vue';
import SettingsBackupRestore from '../pages/SettingsBackupRestore.vue';
import SettingsTime from '../pages/SettingsTime.vue';
import SettingsDebug from '../pages/SettingsDebug.vue';
import MainOverview from '../pages/MainOverview.vue';
import Error404 from '../pages/Error404.vue';

Vue.use(Buefy);
Vue.use(Router);

export default new Router({
  mode:'history',
  linkActiveClass:'is-active',
  routes: [
    {
      path: '/',
      redirect: '/overview'
    },
    {
      path: '/overview',
      component: MainOverview
    },
    {
      path: '/settings',
      redirect: '/settings/network'
    },
    {
      path: '/settings/network',
      component: SettingsNetwork
    },
    {
      path: '/settings/mqtt',
      component: SettingsMQTT
    },
    {
      path: '/settings/time',
      component: SettingsTime
    },
    {
      path: '/settings/backup',
      component: SettingsBackupRestore
    },
    {
      path: '/settings/debug',
      component: SettingsDebug
    },
    {
      path: '/404',
      component: Error404
    },
    {
      path: '*',
      redirect: '/404'
    }
  ]
});
