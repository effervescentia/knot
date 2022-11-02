include Knot.Kore;
include ModuleAliases;

type stream_t = LazyStream.t(Input.t);
type parser_t('a) = stream_t => option(('a, stream_t));
