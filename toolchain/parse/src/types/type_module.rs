use super::type_declaration;
use crate::{matcher as m, module::import};
use combine::{many, parser, skip_many, Parser, Stream};
use lang::ast;

fn type_module_0<T>() -> impl Parser<T, Output = ast::raw::TypeModule>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    let comments = || skip_many(m::comment());

    m::span((
        comments().with(many::<Vec<_>, _, _>(import::import().skip(comments()))),
        many::<Vec<_>, _, _>(type_declaration::type_declaration(type_module).skip(comments())),
    ))
    .map(|((imports, declarations), range)| {
        ast::meta::TypeModule::raw(
            ast::TypeModule {
                imports,
                declarations,
            },
            range,
        )
    })
}

parser! {
    pub fn type_module[T]()(T) -> ast::raw::TypeModule
    where
        [T: Stream<Token = char>, T::Position: m::Position]
    {
        type_module_0()
    }
}

#[cfg(test)]
mod tests {
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> crate::Result<lang::ast::raw::TypeModule> {
        super::type_module().easy_parse(Stream::new(s))
    }

    #[test]
    fn module() {
        let ast = parse(
            "type foo = nil;
view Foo { bar: nil, fizz?: boolean };",
        )
        .unwrap()
        .0;

        insta::assert_debug_snapshot!(ast);
    }
}
