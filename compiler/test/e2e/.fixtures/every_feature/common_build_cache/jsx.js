var $knot = require("@knot/runtime");
var $import$_$views = require("./views");
var PropertiedView = $import$_$views.PropertiedView;
$import$_$views = null;
var propertied_tag_variable = $knot.jsx.createTag("div",
                                                  {
                                                    data_fragment: $knot.jsx.createFragment(),
                                                    data_tag: $knot.jsx.createTag(
                                                    "div"),
                                                    data_string: "abc",
                                                    data_negative_float: -(456.78),
                                                    data_positive_float: 456.78,
                                                    data_negative_integer: -(123),
                                                    data_positive_integer: 123,
                                                    data_false: false,
                                                    data_true: true,
                                                    data_nil: null
                                                  });
exports.propertied_tag_variable = propertied_tag_variable;
var propertied_component_variable = $knot.jsx.createTag(PropertiedView,
                                                        {
                                                          data_fragment: $knot.jsx.createFragment(),
                                                          data_tag: $knot.jsx.createTag(
                                                          "div"),
                                                          data_string: 
                                                          "abc",
                                                          data_negative_float: -(456.78),
                                                          data_positive_float: 456.78,
                                                          data_negative_integer: -(123),
                                                          data_positive_integer: 123,
                                                          data_false: false,
                                                          data_true: true,
                                                          data_nil: null
                                                        });
exports.propertied_component_variable = propertied_component_variable;
var punned_variable = "punned";
exports.punned_variable = punned_variable;
var punned_property_tag_variable = $knot.jsx.createTag("div",
                                                       {
                                                         punned_variable: punned_variable
                                                       });
exports.punned_property_tag_variable = punned_property_tag_variable;
var text_tag_variable = $knot.jsx.createTag("div",
                                            null,
                                            "abc 123 \"def\" %^&#!");
exports.text_tag_variable = text_tag_variable;
var expression_tag_variable = $knot.jsx.createTag("div", null, 123);
exports.expression_tag_variable = expression_tag_variable;
var node_tag_variable = $knot.jsx.createTag("div",
                                            null,
                                            $knot.jsx.createTag("div"));
exports.node_tag_variable = node_tag_variable;
var many_child_tag_variable = $knot.jsx.createTag("div",
                                                  null,
                                                  "abc 123 \"def\" %^&#!",
                                                  123,
                                                  $knot.jsx.createTag(
                                                  "div"));
exports.many_child_tag_variable = many_child_tag_variable;
