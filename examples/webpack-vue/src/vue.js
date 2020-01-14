import Vue from 'vue';

const SomeThing = {
  data: () => ({
    isAltered: false
  }),
  methods: {
    changeTitle() {
      this.isAltered = !this.isAltered;
    }
  },
  render(h) {
    return h('div', { staticClass: 'bubbaledasd' }, [
      this.isAltered ? 'altered title' : 'original title',
      h('button', { on: { click: this.changeTitle } }, 'change it')
    ]);
  }
};

const app = h =>
  h('div', {}, [
    h('h1', {}, ['first', h('SomeThing')]),
    h('h1', {}, 'second'),
    h('h1', {}, 'third')
  ]);

export const main = id => {
  new Vue({
    el: `#${id}`,
    components: {
      SomeThing
    },
    render(h) {
      return app(h);
    }
  });
};
