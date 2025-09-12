#include <iron_net.h>

#import <Foundation/Foundation.h>

static NSURLSession *session = nil;

void iron_https_request(const char *url_base, const char *url_path, const char *data, int port, int method,
                        iron_http_callback_t callback, void *callbackdata) {
	if (session == nil) {
		NSURLSessionConfiguration *sessionConfiguration = [NSURLSessionConfiguration defaultSessionConfiguration];
		session = [NSURLSession sessionWithConfiguration:sessionConfiguration];
	}

	NSString *urlstring = @"https://";
	urlstring = [urlstring stringByAppendingString:[NSString stringWithUTF8String:url_base]];
	urlstring = [urlstring stringByAppendingString:@":"];
	urlstring = [urlstring stringByAppendingString:[[NSNumber numberWithInt:port] stringValue]];
	urlstring = [urlstring stringByAppendingString:@"/"];
	urlstring = [urlstring stringByAppendingString:[NSString stringWithUTF8String:url_path]];
	NSURL *aUrl = [NSURL URLWithString:urlstring];
	NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:aUrl];
	if (data != 0) {
		NSString *datastring = [NSString stringWithUTF8String:data];
		request.HTTPBody = [datastring dataUsingEncoding:NSUTF8StringEncoding];
	}
	request.HTTPMethod = method == IRON_HTTP_GET ? @"GET" : @"POST";

	NSURLSessionDataTask *dataTask = [
		session dataTaskWithRequest:request
		completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
			NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
			dispatch_async(dispatch_get_main_queue(), ^{
				callback((const char *)[data bytes], callbackdata);
			});
		}
	];
	[dataTask resume];
}
