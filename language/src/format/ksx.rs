use crate::ast::{AstNode, KSXNode, KSX};
use kore::format::indented;
use std::fmt::{Display, Formatter, Write};

impl<R, C> Display for KSXNode<R, C>
where
    R: Copy,
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
            self.0.iter().try_fold((), |_, (key, value)| {
                write!(f, " {key}")?;
                if let Some(value) = value {
                    write!(f, "={value}")
                } else {
                    Ok(())
                }
            })
        }
    }
}

impl<R, C> KSXNode<R, C>
where
    R: Copy,
{
    pub fn is_inline(&self) -> bool {
        matches!(self.node().value(), KSX::Text(_) | KSX::Inline(_))
    }
}

struct Children<'a, R, C>(&'a Vec<KSXNode<R, C>>);

impl<'a, R, C> Display for Children<'a, R, C>
where
    R: Copy,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        fn format<R, C, F>(xs: &Vec<KSXNode<R, C>>, mut f: F, all_inline: bool) -> std::fmt::Result
        where
            F: Write,
            R: Copy,
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
                            writeln!(f)?;
                        }
                        _ => (),
                    }

                    write!(f, "{x}")?;
                }

                writeln!(f)?;
            }

            Ok(())
        }

        if self.0.iter().all(KSXNode::is_inline) {
            format(self.0, f, true)
        } else {
            format(self.0, indented(f), false)
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{Expression, Primitive, KSX},
        test::fixture as f,
    };
    use kore::str;

    #[test]
    fn text() {
        assert_eq!(f::n::kx(KSX::Text(str!("foo"))).to_string(), "foo");
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
            f::n::kx(KSX::Fragment(vec![f::n::kx(KSX::Text(str!("foo")))])).to_string(),
            "<>foo</>"
        );
    }

    #[test]
    fn closed_element_no_attributes() {
        assert_eq!(
            f::n::kx(KSX::ClosedElement(str!("foo"), vec![])).to_string(),
            "<foo />"
        );
    }

    #[test]
    fn closed_element_with_attributes() {
        assert_eq!(
            f::n::kx(KSX::ClosedElement(
                str!("foo"),
                vec![
                    (str!("fizz"), None),
                    (
                        str!("buzz"),
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
            f::n::kx(KSX::OpenElement(str!("foo"), vec![], vec![], str!("foo"))).to_string(),
            "<foo></foo>"
        );
    }

    #[test]
    fn open_element_with_attributes() {
        assert_eq!(
            f::n::kx(KSX::OpenElement(
                str!("foo"),
                vec![
                    (str!("fizz"), None),
                    (
                        str!("buzz"),
                        Some(f::n::x(Expression::Primitive(Primitive::Nil)))
                    ),
                ],
                vec![],
                str!("foo"),
            ))
            .to_string(),
            "<foo fizz buzz=nil></foo>"
        );
    }

    #[test]
    fn open_element_with_children() {
        assert_eq!(
            f::n::kx(KSX::OpenElement(
                str!("foo"),
                vec![],
                vec![f::n::kx(KSX::Text(str!("bar")))],
                str!("foo"),
            ))
            .to_string(),
            "<foo>bar</foo>"
        );
    }

    #[test]
    fn element_children() {
        assert_eq!(
            f::n::kx(KSX::OpenElement(
                str!("foo"),
                vec![],
                vec![f::n::kx(KSX::ClosedElement(str!("bar"), vec![],))],
                str!("foo"),
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
                str!("foo"),
                vec![],
                vec![
                    f::n::kx(KSX::Text(str!("hello "))),
                    f::n::kx(KSX::Inline(f::n::x(Expression::Identifier(str!("name"))))),
                    f::n::kx(KSX::Text(str!(", how are you doing?"))),
                    f::n::kx(KSX::ClosedElement(str!("Overview"), vec![],)),
                    f::n::kx(KSX::Inline(f::n::x(Expression::Identifier(str!("left"))))),
                    f::n::kx(KSX::Text(str!(" or "))),
                    f::n::kx(KSX::Inline(f::n::x(Expression::Identifier(str!("right"))))),
                    f::n::kx(KSX::ClosedElement(str!("Summary"), vec![],)),
                    f::n::kx(KSX::Text(str!("that's all folks!"))),
                ],
                str!("foo"),
            ))
            .to_string(),
            "<foo>
  hello {name}, how are you doing?
  <Overview />
  {left} or {right}
  <Summary />
  that's all folks!
</foo>"
        );
    }
}
