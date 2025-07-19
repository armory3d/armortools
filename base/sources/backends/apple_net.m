#include <iron_net.h>

#import <Foundation/Foundation.h>

void iron_http_request(const char *url, const char *path, const char *data, int port, bool secure, int method, const char *header,
                       iron_http_callback_t callback, void *callbackdata) {
	NSString *urlstring = secure ? @"https://" : @"http://";
	urlstring = [urlstring stringByAppendingString:[NSString stringWithUTF8String:url]];
	urlstring = [urlstring stringByAppendingString:@":"];
	urlstring = [urlstring stringByAppendingString:[[NSNumber numberWithInt:port] stringValue]];
	urlstring = [urlstring stringByAppendingString:@"/"];
	urlstring = [urlstring stringByAppendingString:[NSString stringWithUTF8String:path]];

	NSURL *aUrl = [NSURL URLWithString:urlstring];

	NSURLSessionConfiguration *sessionConfiguration = [NSURLSessionConfiguration defaultSessionConfiguration];
	sessionConfiguration.HTTPAdditionalHeaders = @{@"Content-Type" : @"application/json"};
	NSURLSession *session = [NSURLSession sessionWithConfiguration:sessionConfiguration];
	NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:aUrl];
	if (data != 0) {
		// printf("Sending %s\n\n", data);
		NSString *datastring = [NSString stringWithUTF8String:data];
		request.HTTPBody = [datastring dataUsingEncoding:NSUTF8StringEncoding];
	}

	switch (method) {
	case IRON_HTTP_GET:
		request.HTTPMethod = @"GET";
		break;
	case IRON_HTTP_POST:
		request.HTTPMethod = @"POST";
		break;
	case IRON_HTTP_PUT:
		request.HTTPMethod = @"PUT";
		break;
	case IRON_HTTP_DELETE:
		request.HTTPMethod = @"DELETE";
		break;
	}

	NSURLSessionDataTask *dataTask = [session dataTaskWithRequest:request
	                                            completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
		                                            NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
		                                            int statusCode = (int)[httpResponse statusCode];

		                                            NSMutableData *responseData = [[NSMutableData alloc] init];
		                                            [responseData appendData:data];
		                                            [responseData appendBytes:"\0" length:1];

													dispatch_async(dispatch_get_main_queue(), ^{
														callback(error ? 1 : 0, statusCode, (const char *)[responseData bytes], callbackdata);
													});
	                                            }];
	[dataTask resume];
}
