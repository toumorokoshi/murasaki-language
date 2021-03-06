use super::*;

//  #[test]
//  fn test_lexer() {
//      let lexer = Lexer::new();
//      match lexer.symbols.find("+") {
//          Some(t) => {
//              match t {
//                  token::TokenType::Plus => println!("great!"),
//                  _ => assert!(false),
//              }
//          },
//          None => assert!(false),
//      }
//  }
//
//  #[test]
//  fn test_lexer_double_equals() {
//      let lexer = Lexer::new();
//      match lexer.symbols.find("==") {
//          Some(t) => {
//              match t {
//                  token::TokenType::Equal => println!("great!"),
//                  _ => assert!(false),
//              }
//          },
//          None => assert!(false),
//      }
//  }
//
//  #[test]
//  fn test_lexer_double_increment() {
//      let lexer = Lexer::new();
//      match lexer.symbols.find("+=") {
//          Some(t) => {
//              match t {
//                  token::TokenType::Increment => println!("great!"),
//                  _ => assert!(false),
//              }
//          },
//          None => assert!(false),
//      }
// }

#[test]
fn test_lexer_hello_world() {
    let lexer = Lexer::new();
    let output = lexer.read(&"print(\"Hello, World!\")".to_string());
    assert_eq!(output[0].typ, TokenType::Symbol("print".to_string()));
    assert_eq!(output[1].typ, TokenType::ParenL);
    assert_eq!(output[2].typ, TokenType::String("Hello, World!".to_string()));
    assert_eq!(output[3].typ, TokenType::ParenR);
}
