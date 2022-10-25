include Knot.Kore;
include Parse.Onyx;
include ModuleAliases;

module C = Constants;
module M = Grammar.Matchers;

type stream_t = LazyStream.t(Input.t);
type parser_t('a) = stream_t => option(('a, stream_t));
