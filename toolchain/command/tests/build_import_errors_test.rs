mod common;

use lang::Namespace;

#[test]
#[ignore = "skip temporarily"]
fn cyclic() {
    const INPUT_A: &str = "use @/b;";
    const INPUT_B: &str = "use @/c;";
    const INPUT_C: &str = "use @/a;";

    let name = common::test_name(file!(), "absolute_imports");
    let result = common::build(
        &name,
        &[("a.kn", INPUT_A), ("b.kn", INPUT_B), ("c.kn", INPUT_C)],
        web::Web,
    );

    assert_eq!(
        result.unwrap_err().exec_errors().unwrap(),
        &vec![engine::ExecutionError::ImportCycle(vec![
            Namespace::from_internal_path("b"),
            Namespace::from_internal_path("a"),
            Namespace::from_internal_path("c"),
        ])]
    );
}
