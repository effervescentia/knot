import $knot from "@knot/runtime";
var emptyStyle = (function () {
                    var $ = $knot.style.styleExpressionPlugin;
                    var $rules$ = $knot.style.styleRulePlugin;
                    return {};
                  })();
export { emptyStyle };
var colorStyle = (function () {
                    var $ = $knot.style.styleExpressionPlugin;
                    var $rules$ = $knot.style.styleRulePlugin;
                    return {
                      mockColor: $rules$.mockColor($.mockRed)
                    };
                  })();
export { colorStyle };
var rawColorStyle = (function () {
                       var $ = $knot.style.styleExpressionPlugin;
                       var $rules$ = $knot.style.styleRulePlugin;
                       return {
                         mockColor: $rules$.mockColor("#dac33d")
                       };
                     })();
export { rawColorStyle };
