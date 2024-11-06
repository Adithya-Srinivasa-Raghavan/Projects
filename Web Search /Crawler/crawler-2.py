import urllib.parse
import requests
from bs4 import BeautifulSoup
import urllib
from urllib.request import urlopen
import random
from datetime import datetime
import time
import heapq    
from collections import defaultdict

#Extract main domain by parsing the url
def extract_domain(url):
    parsed_url = urllib.parse.urlparse(url)
    domain_parts = parsed_url.netloc.split('.')
    
    if len(domain_parts) >= 2:
        return ".".join(domain_parts[-2:])
    return parsed_url.netloc

#Extract sub-domain by parsing the url
def extract_subdomain(url):
    parsed_url = urllib.parse.urlparse(url)
    domain_parts = parsed_url.netloc.split('.')
    
    if len(domain_parts) > 2:
        return ".".join(domain_parts[:-2])
    return ""

#Ensuring domain in .nz specifically as well as excluding any governmental website(gov sites seem to have a lot of permission issues 
#making the crawler slower and causes errors so not crawling them)
def check_if_nz_domain(url):
    parsed_url = urllib.parse.urlparse(url)
    domain = parsed_url.netloc.lower()
    return ".nz" in domain and not ('.gov' in domain or 'govt' in domain)

#Checking if the url is of html type
def check_if_html_content(grab):
    content_type = grab.headers.get('Content-Type', '')
    return content_type.startswith('text/html') 

#Robot exclusion protocol - Fetching conditionalities or path's which are marked as disallowed in the file
def robot_exclusion_protocol(url):
    disallowed_urls = []
    parsed_url= urllib.parse.urlparse(url)
    robots_txt_file=f"{parsed_url.scheme}://{parsed_url.netloc}/robots.txt"

    try:
        with urlopen(robots_txt_file) as robot_file:
            robots_txt = robot_file.read().decode('UTF-8')
            for line in robots_txt.splitlines():
                line = line.strip()
                if line.startswith('Disallow'):
                    path = line.split(':',1)[1].strip()
                    if path:
                        if path[0]!='/':
                            continue
                        elif path.endswith('*'):
                            path = path[:-1]
                            disallowed_urls.append(f"{parsed_url.scheme}://{parsed_url.netloc}{path}")
    except Exception as e:
        print(f"Error fetching Robots.txt from {url} ==> {e}")
    return disallowed_urls

#Robot exclusion protocol - checking the fetched url's to ensure they can be crawled or if they are disallowed
def check_if_allowed(url, disallowed_urls):
    for disallowed in disallowed_urls:
        if disallowed in url:
            return False
    return True

#Checking the URL to ensure we only crawl webpages/websites and ignores unwanted links
def url_checker(url):
    parsed_url = urllib.parse.urlparse(url)
    valid_page_extensions = ('.htm', '.html', '.php', '.jsp', '.asp', '.aspx')
    invalid_extensions = ('.png', '.jpg', '.jpeg', '.gif', '.pdf', '.mp4', '.avi', '.mov', '.mp3', '.xml', '.css', '.js')
    path = parsed_url.path.lower()
    return (path.endswith(valid_page_extensions) or not any(ext in path for ext in invalid_extensions))

#Taking Metrics for the Crawler
def log_metrics(metrics):
    with open("stats.txt", "a") as metrics_file:
        metrics_file.write(f"Total Pages Visited = {metrics['visited_pages']}\n")
        metrics_file.write(f"Total Errors = {metrics['error_count']}\n")
        metrics_file.write(f"Total Non webpages = {metrics['not-webpage']}\n")
        metrics_file.write(f"Total Page Size = {metrics['page_size']}\n")

#Main Logic which contains the following - 
#Set's for each domain and sub-domain visited, Priority Queue, A robot cache to try and minimize number of times the robots file is checked for the url's, 
#metrics counter for the stats and the time. 
def process_seed_list(seed_list, filename, crawl_duration, log_interval):
    visited_urls = set()
    priority_queue=[]
    robots_cache = {}
    metrics = defaultdict(int)
    start_time = time.time()
    last_log_time = start_time
    
    for url in seed_list:
        heapq.heappush(priority_queue, (0, 1, url))

    while priority_queue and (time.time() - start_time) < crawl_duration:
        priority, depth, url = heapq.heappop(priority_queue)

        if url in visited_urls:
            continue
        visited_urls.add(url)

        domain = extract_domain(url)
        subdomain = extract_subdomain(url)

        try:
            if not check_if_nz_domain(url):
                continue
            if domain not in robots_cache:
                robots_cache[domain] = robot_exclusion_protocol(url)
            disallowed_urls = robots_cache[domain]
            if not check_if_allowed(url, disallowed_urls):
                print(f"URL Disallowed by robots.txt - {url}")
                continue

            grab = requests.get(url, timeout=5)
            grab.raise_for_status()
            
            if not check_if_html_content(grab) or not url_checker(url):
                metrics['not-webpage'] +=1
                continue
            
            response_time = time.time() - start_time
            page_size = len(grab.content)

            metrics['visited_pages'] +=1
            metrics['time_for_response'] +=response_time
            metrics['page_size'] +=page_size

            soup = BeautifulSoup(grab.text, 'html.parser')
        
            with open(filename, "a") as f:
                crawl_time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))
                return_status = grab.status_code
                f.write(f" URL: {url}, Depth: {depth}, Priority: {priority}, Crawled at: {crawl_time}, Page Size: {page_size} bytes, Return Code: {return_status} \n")

                for link in soup.find_all("a"):
                    data = link.get("href")
                    try:
                        if data:
                            full_url = urllib.parse.urljoin(url, data.strip())
                            parsed_url = urllib.parse.urlparse(full_url)
                            if parsed_url.scheme in ["http", "https"] and not full_url.startswith(("mailto:", "javascript", "#")):
                                new_domain = extract_domain(full_url)
                                new_subdomain = extract_subdomain(full_url)
                                if new_domain not in robots_cache:
                                    robots_cache[new_domain] = robot_exclusion_protocol(full_url)
                                new_disallowed_urls = robots_cache[new_domain]
                                
                                if full_url not in visited_urls and check_if_allowed(full_url, new_disallowed_urls) and check_if_nz_domain(full_url) and url_checker(full_url):
                                    heapq.heappush(priority_queue, (0, depth + 1, full_url))
                                f.write(f"{full_url}\n")
                    except (ValueError, TypeError) as error:  
                        metrics['error_count'] += 1
                        print(f"Error for this url --> {full_url} and it looks like this --> {error}")

        except requests.exceptions.RequestException as error:
            metrics['error_count'] += 1
            print(f"Error fetching {url}: {error}")    
        
        if (time.time() - last_log_time) >= log_interval:
            log_metrics(metrics)
            last_log_time = time.time()

#Randomly select 20 sites from the seed list for each run, run for the given time and print stats in the interval specified       
def main():
    
    seed_list_1 = []
    seed_list_2 = []

    with open('nz_domain_seeds_list.txt' , 'r') as seed_sites_list:
        sites = seed_sites_list.read().splitlines()

    random.shuffle(sites)
    seed_list_1 = sites[:20]
    seed_list_2 = sites[20:40]

    process_seed_list(seed_list_1, "test1.txt", crawl_duration=7200, log_interval=300)
    process_seed_list(seed_list_2, "test2.txt", crawl_duration=7200, log_interval=300)

if __name__ == "__main__":
    main()