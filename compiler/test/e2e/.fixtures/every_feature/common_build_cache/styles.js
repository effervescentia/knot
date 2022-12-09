var $knot = require("@knot/runtime");
var emptyStyle = (function () {
                    var $ = $knot.style.styleExpressionPlugin;
                    var $rules$ = $knot.style.styleRulePlugin;
                    return $knot.style.createStyle({});
                  })();
exports.emptyStyle = emptyStyle;
var redStyle = (function () {
                  var $ = $knot.style.styleExpressionPlugin;
                  var $rules$ = $knot.style.styleRulePlugin;
                  return $knot.style.createStyle({
                                                   mockColor: $rules$.mockColor(
                                                   $.mockRed)
                                                 });
                })();
exports.redStyle = redStyle;
var yellowStyle = (function () {
                     var $ = $knot.style.styleExpressionPlugin;
                     var $rules$ = $knot.style.styleRulePlugin;
                     return $knot.style.createStyle({
                                                      mockBackgroundColor: $rules$.mockBackgroundColor(
                                                      "#dac33d")
                                                    });
                   })();
exports.yellowStyle = yellowStyle;
function Component($props$) {
  return null;
};
exports.Component = Component;
var EmptyComponent = $knot.style.bindStyle(Component, emptyStyle);
exports.EmptyComponent = EmptyComponent;
var RedAndYellowComponent = $knot.style.bindStyle($knot.style.bindStyle(
                                                  Component,
                                                  redStyle),
                                                  yellowStyle);
exports.RedAndYellowComponent = RedAndYellowComponent;
var StyleLiteralComponent = $knot.style.bindStyle(Component,
                                                  (function () {
                                                     var $ = $knot.style.styleExpressionPlugin;
                                                     var $rules$ = $knot.style.styleRulePlugin;
                                                     return $knot.style.createStyle(
                                                     {
                                                       mockColor: $rules$.mockColor(
                                                       "#adec22")
                                                     });
                                                   })());
exports.StyleLiteralComponent = StyleLiteralComponent;
function DynamicStyling($props$) {
  return $knot.jsx.createTag("div",
                             {
                               className: $knot.style.classes(redStyle.getClass())
                             },
                             $knot.jsx.createTag("div",
                                                 {
                                                   className: $knot.style.classes(
                                                   yellowStyle.getClass())
                                                 }),
                             $knot.jsx.createTag("div",
                                                 {
                                                   className: $knot.style.classes(
                                                   (function () {
                                                      var $ = $knot.style.styleExpressionPlugin;
                                                      var $rules$ = $knot.style.styleRulePlugin;
                                                      return $knot.style.createStyle(
                                                      {
                                                        mockColor: $rules$.mockColor(
                                                        "#adec22")
                                                      });
                                                    })().getClass())
                                                 }));
};
exports.DynamicStyling = DynamicStyling;
