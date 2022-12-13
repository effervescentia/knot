import $knot from "@knot/runtime";
import { PartialView, PropertiedView } from "./views";
var propertied_tag_variable = $knot.jsx.createTag("data_element",
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
export { propertied_tag_variable };
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
export { propertied_component_variable };
var optional_properties_excluded = $knot.jsx.createTag(PartialView,
                                                       {
                                                         first: 100
                                                       });
export { optional_properties_excluded };
var optional_properties_included = $knot.jsx.createTag(PartialView,
                                                       {
                                                         second: true,
                                                         first: 100
                                                       });
export { optional_properties_included };
var punned_variable = "punned";
export { punned_variable };
var punned_property_tag_variable = $knot.jsx.createTag("span",
                                                       {
                                                         punned_variable: punned_variable
                                                       });
export { punned_property_tag_variable };
var text_tag_variable = $knot.jsx.createTag("div",
                                            null,
                                            "abc 123 \"def\" %^&#!");
export { text_tag_variable };
var expression_tag_variable = $knot.jsx.createTag("div", null, 123);
export { expression_tag_variable };
var node_tag_variable = $knot.jsx.createTag("div",
                                            null,
                                            $knot.jsx.createTag("div"));
export { node_tag_variable };
var many_child_tag_variable = $knot.jsx.createTag("div",
                                                  null,
                                                  "abc 123 \"def\" %^&#!",
                                                  123,
                                                  $knot.jsx.createTag(
                                                  "div"));
export { many_child_tag_variable };
