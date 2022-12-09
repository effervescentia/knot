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
                                                      mockBackgroundColor: 
                                                      "#dac33d"
                                                    });
                   })();
exports.yellowStyle = yellowStyle;
function Component($props$) {
  return null;
};
exports.Component = Component;
var EmptyComponent = $knot.jsx.bindStyle(Component, emptyStyle);
exports.EmptyComponent = EmptyComponent;
var RedAndYellowComponent = $knot.jsx.bindStyle($knot.jsx.bindStyle(Component,
                                                                    redStyle),
                                                yellowStyle);
exports.RedAndYellowComponent = RedAndYellowComponent;
var StyleLiteralComponent = $knot.jsx.bindStyle(Component,
                                                (function () {
                                                   var $ = $knot.style.styleExpressionPlugin;
                                                   var $rules$ = $knot.style.styleRulePlugin;
                                                   return $knot.style.createStyle(
                                                   {
                                                     mockColor: "#adec22"
                                                   });
                                                 })());
exports.StyleLiteralComponent = StyleLiteralComponent;
var redDiv = $knot.jsx.bindStyle("div", redStyle);
exports.redDiv = redDiv;
function DynamicStyling($props$) {
  return $knot.jsx.createTag(redDiv,
                             null,
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
                                                        mockColor: "#adec22"
                                                      });
                                                    })().getClass())
                                                 }));
};
exports.DynamicStyling = DynamicStyling;
