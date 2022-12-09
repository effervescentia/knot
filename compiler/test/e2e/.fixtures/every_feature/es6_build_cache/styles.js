import $knot from "@knot/runtime";
var emptyStyle = (function () {
                    var $ = $knot.style.styleExpressionPlugin;
                    var $rules$ = $knot.style.styleRulePlugin;
                    return $knot.style.createStyle({});
                  })();
export { emptyStyle };
var redStyle = (function () {
                  var $ = $knot.style.styleExpressionPlugin;
                  var $rules$ = $knot.style.styleRulePlugin;
                  return $knot.style.createStyle({
                                                   mockColor: $rules$.mockColor(
                                                   $.mockRed)
                                                 });
                })();
export { redStyle };
var yellowStyle = (function () {
                     var $ = $knot.style.styleExpressionPlugin;
                     var $rules$ = $knot.style.styleRulePlugin;
                     return $knot.style.createStyle({
                                                      mockBackgroundColor: 
                                                      "#dac33d"
                                                    });
                   })();
export { yellowStyle };
function Component($props$) {
  return null;
};
export { Component };
var EmptyComponent = $knot.jsx.bindStyle(Component, emptyStyle);
export { EmptyComponent };
var RedAndYellowComponent = $knot.jsx.bindStyle($knot.jsx.bindStyle(Component,
                                                                    redStyle),
                                                yellowStyle);
export { RedAndYellowComponent };
var StyleLiteralComponent = $knot.jsx.bindStyle(Component,
                                                (function () {
                                                   var $ = $knot.style.styleExpressionPlugin;
                                                   var $rules$ = $knot.style.styleRulePlugin;
                                                   return $knot.style.createStyle(
                                                   {
                                                     mockColor: "#adec22"
                                                   });
                                                 })());
export { StyleLiteralComponent };
var redDiv = $knot.jsx.bindStyle("div", redStyle);
export { redDiv };
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
export { DynamicStyling };
