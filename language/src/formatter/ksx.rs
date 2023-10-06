use super::indented;
use crate::{
    ast::ksx::{KSXNode, KSX},
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::{Debug, Display, Formatter, Write};

impl<T, C> Display for KSXNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self.node().value() {
            KSX::Text(x) => write!(f, "{x}"),

            KSX::Inline(x) => write!(f, "{{{x}}}"),

            KSX::Fragment(children) => write!(f, "<>{children}</>", children = Children(children)),

            KSX::ClosedElement(tag, attributes) => write!(
                f,
                "<{tag}{attributes} />",
                attributes = Attributes(attributes)
            ),

            KSX::OpenElement(start_tag, attributes, children, end_tag) => {
                write!(
                    f,
                    "<{start_tag}{attributes}>{children}</{end_tag}>",
                    attributes = Attributes(attributes),
                    children = Children(children)
                )
            }
        }
    }
}

struct Attributes<'a, T>(&'a Vec<(String, Option<T>)>)
where
    T: Display;

impl<'a, T> Display for Attributes<'a, T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        if self.0.is_empty() {
            Ok(())
        } else {
            self.0.iter().fold(Ok(()), |acc, (key, value)| {
                acc.and_then(|_| {
                    write!(f, " {key}")?;
                    if let Some(value) = value {
                        write!(f, "={value}")
                    } else {
                        Ok(())
                    }
                })
            })
        }
    }
}

impl<T, C> KSXNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn is_inline(&self) -> bool {
        match self.node().value() {
            KSX::Text(_) | KSX::Inline(_) => true,
            _ => false,
        }
    }
}

struct Children<'a, T, C>(&'a Vec<KSXNode<T, C>>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<'a, T, C> Display for Children<'a, T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        fn format<'a, T, C, F>(
            xs: &'a Vec<KSXNode<T, C>>,
            mut f: F,
            all_inline: bool,
        ) -> std::fmt::Result
        where
            T: Stream<Token = char>,
            T::Position: Copy + Debug + Decrement,
            F: Write,
        {
            if all_inline {
                for x in xs {
                    write!(f, "{x}")?;
                }
            } else {
                let mut prev_inline = None;

                for x in xs {
                    let next_inline = x.is_inline();

                    match (prev_inline, next_inline) {
                        (_, false) | (Some(false), true) | (None, _) => {
                            prev_inline = Some(next_inline);
                            write!(f, "\n")?;
                        }
                        _ => (),
                    }

                    write!(f, "{x}")?;
                }

                write!(f, "\n")?;
            }

            Ok(())
        }

        if self.0.iter().all(|x| x.is_inline()) {
            format(self.0, f, true)
        } else {
            format(self.0, indented(f), false)
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{
            expression::{Expression, Primitive},
            ksx::KSX,
        },
        test::fixture as f,
    };

    #[test]
    fn text() {
        assert_eq!(f::n::kx(KSX::Text(String::from("foo"))).to_string(), "foo");
    }

    #[test]
    fn inline_expression() {
        assert_eq!(
            f::n::kx(KSX::Inline(f::n::x(Expression::Primitive(Primitive::Nil)))).to_string(),
            "{nil}"
        );
    }

    #[test]
    fn empty_fragment() {
        assert_eq!(f::n::kx(KSX::Fragment(vec![])).to_string(), "<></>");
    }

    #[test]
    fn fragment_with_children() {
        assert_eq!(
            f::n::kx(KSX::Fragment(vec![f::n::kx(KSX::Text(String::from(
                "foo"
            )))]))
            .to_string(),
            "<>foo</>"
        );
    }

    #[test]
    fn closed_element_no_attributes() {
        assert_eq!(
            f::n::kx(KSX::ClosedElement(String::from("foo"), vec![])).to_string(),
            "<foo />"
        );
    }

    #[test]
    fn closed_element_with_attributes() {
        assert_eq!(
            f::n::kx(KSX::ClosedElement(
                String::from("foo"),
                vec![
                    (String::from("fizz"), None),
                    (
                        String::from("buzz"),
                        Some(f::n::x(Expression::Primitive(Primitive::Nil)))
                    ),
                ]
            ))
            .to_string(),
            "<foo fizz buzz=nil />"
        );
    }

    #[test]
    fn empty_open_element() {
        assert_eq!(
            f::n::kx(KSX::OpenElement(
                String::from("foo"),
                vec![],
                vec![],
                String::from("foo")
            ))
            .to_string(),
            "<foo></foo>"
        );
    }

    #[test]
    fn open_element_with_attributes() {
        assert_eq!(
            f::n::kx(KSX::OpenElement(
                String::from("foo"),
                vec![
                    (String::from("fizz"), None),
                    (
                        String::from("buzz"),
                        Some(f::n::x(Expression::Primitive(Primitive::Nil)))
                    ),
                ],
                vec![],
                String::from("foo"),
            ))
            .to_string(),
            "<foo fizz buzz=nil></foo>"
        );
    }

    #[test]
    fn open_element_with_children() {
        assert_eq!(
            f::n::kx(KSX::OpenElement(
                String::from("foo"),
                vec![],
                vec![f::n::kx(KSX::Text(String::from("bar")))],
                String::from("foo"),
            ))
            .to_string(),
            "<foo>bar</foo>"
        );
    }

    #[test]
    fn element_children() {
        assert_eq!(
            f::n::kx(KSX::OpenElement(
                String::from("foo"),
                vec![],
                vec![f::n::kx(KSX::ClosedElement(String::from("bar"), vec![],))],
                String::from("foo"),
            ))
            .to_string(),
            "<foo>
  <bar />
</foo>"
        );
    }

    #[test]
    fn mixed_children() {
        assert_eq!(
            f::n::kx(KSX::OpenElement(
                String::from("foo"),
                vec![],
                vec![f::n::kx(KSX::Text(String::from("bar")))],
                String::from("foo"),
            ))
            .to_string(),
            "<foo>bar</foo>"
        );
    }
}
