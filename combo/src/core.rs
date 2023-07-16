pub struct Stream {
    iter: dyn Iterator<Item = char>,
}

pub fn return_<T>(value: T, stream: &mut Stream) -> Option<(T, &mut Stream)> {
    Some((value, stream))
}

pub fn any(stream: &mut Stream) -> Option<(char, &mut Stream)> {
    match stream.iter.next() {
        Some(result) => Some((result, stream)),
        None => None,
    }
}

pub fn eof<T>(value: T, stream: &mut Stream) -> Option<(T, &Stream)> {
    match stream.iter.next() {
        None => Some((value, stream)),
        Some(_) => None,
    }
}

/// (>>=)
///
/// if [parser] succeeds, returns a parser defined by calling [factory] with the result of [parser]
pub fn bind<
    R1,
    R2,
    F1: Fn(&mut Stream) -> Option<(R1, &mut Stream)>,
    F2: Fn(R1, &mut Stream) -> Option<(R2, &mut Stream)>,
>(
    parser: F1,
    factory: F2,
    stream: &mut Stream,
) -> Option<(R2, &mut Stream)> {
    match parser(stream) {
        Some((result, stream)) => factory(result, stream),
        None => None,
    }
}

/// (<<!)
///
/// if [parser] succeeds, attempt to parse with [success]
/// if [parser] fails return the result of [parser] without advancing the stream
pub fn negative_lookahead<
    'a,
    R1: Copy,
    R2,
    F1: Fn(&mut Stream) -> Option<(R1, &mut Stream)>,
    F2: Fn(R1) -> Option<(R1, &'a Stream)>,
>(
    parser: F1,
    success: F2,
    stream: &mut Stream,
) -> Option<(R1, &mut Stream)> {
    bind(
        parser,
        |result, stream| match success(result) {
            Some(_) => None,
            None => Some((result, stream)),
        },
        stream,
    )
}

/// (>|=)
///
/// if [parser] succeeds, transform the result with [transform]
pub fn map_result<R1, R2, F1: Fn(&mut Stream) -> Option<(R1, &mut Stream)>, F2: Fn(R1) -> R2>(
    parser: F1,
    transform: F2,
    stream: &mut Stream,
) -> Option<(R2, &mut Stream)> {
    bind(
        parser,
        |result, stream| return_(transform(result), stream),
        stream,
    )
}

/// (>@=)
///
/// if [parser] succeeds, execute [effect] with result then return result
pub fn produce_effect<
    R1: Copy,
    F1: Fn(&mut Stream) -> Option<(R1, &mut Stream)>,
    F2: Fn(R1) -> (),
>(
    parser: F1,
    effect: F2,
    stream: &mut Stream,
) -> Option<(R1, &mut Stream)> {
    bind(
        parser,
        |result, stream| {
            effect(result);
            return_(result, stream)
        },
        stream,
    )
}

/// (<*>)
///
/// if [parser] succeeds, use the function it returns to map the result of [success]
// pub fn apply<
//     R1: Fn(R2) -> R3,
//     R2,
//     R3,
//     F1: Fn(&mut Stream) -> Option<(R1, &mut Stream)>,
//     F2: Fn(&mut Stream) -> Option<(R2, &mut Stream)>,
// >(
//     parser: F1,
//     success: F2,
//     stream: &mut Stream,
// ) -> Option<(R3, &mut Stream)> {
//     bind(
//         parser,
//         |transform, stream| map_result(success, transform, stream),
//         stream,
//     )
// }

/// (>>)
///
/// if [parser] succeeds, drop the result and try to parse with [next]
pub fn discard_left<
    R1,
    R2,
    F1: Fn(&mut Stream) -> Option<(R1, &mut Stream)>,
    F2: Fn(&mut Stream) -> Option<(R2, &mut Stream)>,
>(
    parser: F1,
    next: F2,
    stream: &mut Stream,
) -> Option<(R2, &mut Stream)> {
    bind(parser, |_, stream| next(stream), stream)
}

/// (<<)
///
/// if [parser] succeeds, attempt to parse with [next] but return the original result
// pub fn discard_right<
//     R1,
//     R2,
//     F1: Fn(&mut Stream) -> Option<(R1, &mut Stream)>,
//     F2: Fn(&mut Stream) -> Option<(R2, &mut Stream)>,
// >(
//     parser: F1,
//     next: F2,
//     stream: &mut Stream,
// ) -> Option<(R2, &mut Stream)> {
//     bind(parser, |_, stream| next(stream), stream)
// }

/// (<|>)
///
/// if [parser] succeeds, return the result, otherwise attempt to parse with [next]
// pub fn else_<
//     R1,
//     F1: Fn(&mut Stream) -> Option<(R1, &mut Stream)>,
//     F2: Fn(&mut Stream) -> Option<(R1, &mut Stream)>,
// >(
//     parser: F1,
//     next: F2,
//     stream: &mut Stream,
// ) -> Option<(R1, &mut Stream)> {
//     match parser(stream) {
//         result @ Some(_) => result,
//         None => next(stream),
//     }
// }

/// (<?>)
///
/// if [parser] succeeds, return the result, otherwise throw [error]
pub fn finally<
    R1,
    F1: Fn(&mut Stream) -> Option<(R1, &mut Stream)>,
    F2: Fn(&mut Stream) -> Option<(R1, &mut Stream)>,
>(
    parser: F1,
    stream: &mut Stream,
) -> Option<(R1, &mut Stream)> {
    match parser(stream) {
        result @ Some(_) => result,
        None => None,
    }
}

/// (<~>)
///
/// if [parse_first] succeeds, prepend the result to the results of [parse_rest]
pub fn chain<
    R1: Copy,
    F1: Fn(&mut Stream) -> Option<(R1, &mut Stream)>,
    F2: Fn(&mut Stream) -> Option<(&[R1], &mut Stream)>,
>(
    parse_first: F1,
    parse_rest: F2,
    stream: &mut Stream,
) -> Option<(Vec<R1>, &mut Stream)> {
    bind(
        parse_first,
        |first, stream| {
            map_result(
                parse_rest,
                |rest| {
                    let all = vec![first];
                    all.append(rest.clone());
                    all
                },
                stream,
            )
        },
        stream,
    )
}
