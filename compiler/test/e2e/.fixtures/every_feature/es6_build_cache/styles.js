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
                                                      mockBackgroundColor: $rules$.mockBackgroundColor(
                                                      "#dac33d")
                                                    });
                   })();
export { yellowStyle };
function Component($props$) {
  return null;
};
export { Component };
var EmptyComponent = $knot.style.bindStyle(Component, emptyStyle);
export { EmptyComponent };
var RedAndYellowComponent = $knot.style.bindStyle($knot.style.bindStyle(
                                                  Component,
                                                  redStyle),
                                                  yellowStyle);
export { RedAndYellowComponent };
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
export { StyleLiteralComponent };
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
export { DynamicStyling };
