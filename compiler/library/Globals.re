include Exception;
include Invariant;
include Operators;

module LazyStream = Opal.LazyStream;
module Log = Log;
module Emoji = Emoji;

type cursor = (int, int);
type uchar_stream = LazyStream.t((Uchar.t, cursor));
