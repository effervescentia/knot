import { Application } from 'typedoc/dist/lib/application';
import { ConverterComponent } from 'typedoc/dist/lib/converter/components';
import { RendererComponent } from 'typedoc/dist/lib/output/components';
import { Component } from 'typedoc/dist/lib/utils';

const RENDERER_NAME = 'knot_renderer';
const CONVERTER_NAME = 'knot_converter';

@Component({ name: RENDERER_NAME })
class Renderer extends RendererComponent {}

@Component({ name: CONVERTER_NAME })
class Converter extends ConverterComponent {}

export = ({ owner: app }: Application) => {
  if (!app.renderer.hasComponent(RENDERER_NAME)) {
    app.renderer.addComponent(RENDERER_NAME, new Renderer(app.renderer));
  }

  if (!app.converter.hasComponent(CONVERTER_NAME)) {
    app.converter.addComponent(CONVERTER_NAME, new Converter(app.converter));
  }
};
